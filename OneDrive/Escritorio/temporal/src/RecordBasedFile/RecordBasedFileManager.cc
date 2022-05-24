
#include "RecordBasedFileManager.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>

/***
 * Códigos de error retornados por RecordBasedFileManager:
 * -20 = tipo de registro desconocido en función printRecord
 * -21 = tamaño de registro excede el tamaño de página
 * -22 = registro no pudo ser insertado, información en header es incorrecta
 * -23 = rid no está correctamente configurado O el número de página excede el total de páginas en un archivo
 * -24 = directorio de slots almacena información incorrecta
 * -25 = actualización de registro no puede cambiar el rid
 * -26 = atributo solicitado no fue encontrado en registro
***/

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;
PagedFileManager* RecordBasedFileManager::_pfm = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
    {
        _rbf_manager = new RecordBasedFileManager();

        // crear una instancia de PagedFileManager
        _pfm = PagedFileManager::instance();
    }

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}



RC RecordBasedFileManager::createFile(const string &fileName) {

    RC errCode = 0;

    // crear un archivo vacío usando pfm (PagedFileManager)
    if( (errCode = _pfm->createFile( fileName.c_str() )) != 0 )
    {
        return errCode;
    }

    /***
     * Creación de directorio de páginas en archivo:
     * Un directorio de páginas es usado, el cual usa una lista enlazada de headers de página
     * El contenido del header de página es el siguiente:
     *   + nextHeaderPageId:PageNum
     *   + numUsedPageIds:PageNum (int)
     *   + arreglo de <pageIds:(unsigned integer), numFreeBytes:(unsigned integer)>
     *
     *   para el resto del header de página => (unsigned integer)[PAGE_SIZE - 2*SIZEOF(pageNum)]
    ***/
    FileHandle fileHandle;
    errCode = _pfm->openFile(fileName.c_str(), fileHandle);
    if(errCode != 0)
    {
        return errCode;
    }

    // crear header
    Header header;

    // setear todos los campos del header a 0
    memset(&header, 0, sizeof(Header));

    // insertar header en el archivo
    fileHandle.appendPage(&header);

    // cerrar archivo
    errCode = _pfm->closeFile(fileHandle);
    if(errCode != 0)
    {
        return errCode;
    }

    // éxito, retornar 0
    return errCode;
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
    RC errCode = 0;

    // eliminar archivo
    if( (errCode = _pfm->destroyFile( fileName.c_str() )) != 0 )
    {
        return errCode;
    }

    // éxito, retornar 0
    return errCode;
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
    RC errCode = 0;

    // abrir archivo y asignar manejador de archivo
    if( (errCode = _pfm->openFile( fileName.c_str(), fileHandle )) != 0 )
    {
        return errCode;
    }

    // éxito, retornar 0
    return errCode;
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    RC errCode = 0;

    // cerrar archivo
    if( (errCode = _pfm->closeFile( fileHandle )) != 0 )
    {
        return errCode;
    }

    // éxito, retornar 0
    return errCode;
}

RC RecordBasedFileManager::getDataPage(FileHandle &fileHandle, const unsigned int recordSize, PageNum& pageNum, PageNum& headerPage, unsigned int& freeSpaceLeftInPage)
{
    RC errCode = 0;

    // mantener id de header de página actual y su valor desde iteración anterior
    unsigned int headerPageId = 0, lastHeaderPageId = 0;

    // mantener arreglo de tamaños (en bytes) de la página para su lectura
    void* data = malloc(PAGE_SIZE);

    // castear data a header de página
    Header* hPage = NULL;

    // iterear sobre los headers de página
    do
    {
        // obtener el primer header de página
        if( (errCode = fileHandle.readPage((PageNum)headerPageId, data)) != 0 )
        {
            // liberar data
            free(data);

            return errCode; // error
        }

        // castear data a header de página
        hPage = (Header*)data;

        // obtener el id del siguiente header de página
        lastHeaderPageId = headerPageId;
        headerPageId = hPage->_nextHeaderPageId;

        // iterar sobre las entradas de páginas de datos del actual header de página
        for( unsigned int i = 0; i < hPage->_numUsedPageIds; i++ )
        {
            // obtener la información de la página de datos actual
            PageInfo* pi = &(hPage->_arrOfPageIds[i]);

            // Si esta tiene el número de bytes disponibles considerando el tamaño de registro solicitado
            if( pi->_numFreeBytes > recordSize + sizeof(PageDirSlot) )
            {
                // asignar id
                pageNum = pi->_pageid;

                // actualizar el espacio disponible en bytes
                pi->_numFreeBytes -= recordSize + sizeof(PageDirSlot);

                freeSpaceLeftInPage = pi->_numFreeBytes;

                // asignar un header de página
                headerPage = lastHeaderPageId;

                // escribir el header de página
                fileHandle.writePage(headerPage, data);

                // liberar data
                free(data);

                return 0; // operación exitosa
            }
        }

    } while(headerPageId > 0);

    // asignar el id de header de página
    headerPage = lastHeaderPageId;

    // verificar si el último header de página procesado NO puede  almacenar el nuevo registro
    if( hPage->_numUsedPageIds >= NUM_OF_PAGE_IDS )
    {
        // este header de página está lleno, es necesario crear uno nuevo
        // asignar nuevo id de header de página
        PageNum nextPageId = fileHandle.getNumberOfPages();

        // asignar nuevo header de página
        hPage->_nextHeaderPageId = nextPageId;

        // guardar header de página
        fileHandle.writePage(headerPage, data);

        headerPage = nextPageId;

        // preparar parámetros para asignar header de página
        memset(data, 0, PAGE_SIZE);

        // añadir nuevo header de página
        if( (errCode = fileHandle.appendPage(data)) != 0 )
        {
            // liberar data
            free(data);

            return errCode; // error
        }

        // todos los campos del header de página son accesados mediante la referenciación a la estrucutra Header
        hPage = (Header*)data;
    }

    // se necesita crear nueva página de datos, esto es porque todos los páginas de datos no tiene suficiente espacio
    // preparar parámetros para la asignación de nueva página de datos
    void* dataPage = malloc(PAGE_SIZE);
    memset((void*)dataPage, 0, PAGE_SIZE);

    // crear una nueva página de datos
    if( (errCode = fileHandle.appendPage(dataPage)) != 0 )
    {
        // liberar data y dataPage
        free(data);
        free(dataPage);

        return errCode; // retornar error
    }

    // obtener id de la nueva página
    PageNum dataPageId = fileHandle.getNumberOfPages() - 1;	//page indexes are off by extra 1, header page is '0' and first data page is '1'!

    // setear el nuevo registro
    hPage->_arrOfPageIds[hPage->_numUsedPageIds]._pageid = dataPageId;
    if( hPage->_arrOfPageIds[hPage->_numUsedPageIds]._numFreeBytes == 0 )
        hPage->_arrOfPageIds[hPage->_numUsedPageIds]._numFreeBytes = PAGE_SIZE - 2 * sizeof(unsigned int);
    hPage->_arrOfPageIds[hPage->_numUsedPageIds]._numFreeBytes -= recordSize + sizeof(PageDirSlot);

    freeSpaceLeftInPage = hPage->_arrOfPageIds[hPage->_numUsedPageIds]._numFreeBytes;

    // asignar id de página de datos
    pageNum = dataPageId;

    // incrementar el número de ids de páginas usados
    hPage->_numUsedPageIds++;

    // escribir header de página
    fileHandle.writePage(headerPage, data);

    // escribir página de datos
    fileHandle.writePage(dataPageId, dataPage);

    // liberar data y dataPage
    free(data);
    free(dataPage);

    return 0; // operación exitosa
}

RC RecordBasedFileManager::findRecordSlot(FileHandle &fileHandle, PageNum pagenum, const unsigned int szRecord, PageDirSlot& slot, unsigned int& slotNum, unsigned int freeSpaceLeftInPage)
{
    RC errCode = 0;

    // preparar parámetros para leer página de datos
    void* data = malloc(PAGE_SIZE);
    memset(data, 0, PAGE_SIZE);

    // obtener contenido de página de datos
    if( (errCode = fileHandle.readPage(pagenum, data)) != 0 )
    {
        // liberar data
        free(data);

        return errCode; // retornar error
    }

    /***
     * Página de datos tiene el siguiente formato:
     * [lista de registros sin espacio de separación][espacio libre para nuevos registros][directorio slots][número de slots:(unsigned int)][offset desde inicio de página al inicio del espacio libre:unsigned int]
     * ^                                                                                  ^                ^                                                                                                       ^
     * inicio de página                                                         inicio de dirSlot      fin de dirSlot                                                                                  fin de página
     ***/

    // determinar puntero al final del directorio de slots
    PageDirSlot* endOfDirSlot = (PageDirSlot*)((char*)data + PAGE_SIZE - sizeof(unsigned int) - sizeof(unsigned int));

    // número de slots
    unsigned int* ptrNumSlots = (unsigned int*)(endOfDirSlot);

    // puntero al inicio del espacio libre
    char* ptrToFreeSpace = (char*)((char*)data + *( (unsigned int*)( (char*)(endOfDirSlot) + sizeof(unsigned int) ) ));
    unsigned int* ptrVarForFreeSpace = (unsigned int*)( (char*)(endOfDirSlot) + sizeof(unsigned int) );

    // puntero al inicio del directorio de slots
    PageDirSlot* startOfDirSlot = (PageDirSlot*)( endOfDirSlot - (*ptrNumSlots) );

    // determinar si hay algún slot disponible en el directorio de slots, por ej. de registros eliminados previamente
    PageDirSlot* curSlot = startOfDirSlot;
    while(curSlot != endOfDirSlot)
    {
        // verificar si este slot está vació: (unsigned int*)( (char*)(endOfDirSlot) + sizeof(unsigned int) )
        if(curSlot->_offRecord == 0 && curSlot->_szRecord == 0)
        {
            // slot vació encontrado
            break;
        }

        // incrementar al siguiente slot
        curSlot++;
    }

    // si no hay slot disponible en el directorio de slots, entonces reservar siguiente (obtener un puntero a el)
    if( curSlot == endOfDirSlot )
    {
        // apuntar al slot antes del comienzo del directorio de slots
        curSlot = (PageDirSlot*)(startOfDirSlot - 1);
        startOfDirSlot = curSlot;

        // inicializar a 0's
        curSlot->_offRecord = 0;
        curSlot->_szRecord = 0;

        // incrementar el numero de slots: (unsigned int*)( (char*)(endOfDirSlot) + sizeof(unsigned int) )
        *ptrNumSlots += 1;
    }

    // determinar el espacio libre en esta página
    unsigned int szOfFreeSpace = (unsigned int)((char*)startOfDirSlot - ptrToFreeSpace);

    if( freeSpaceLeftInPage != (szOfFreeSpace - szRecord) )
    {
        freeSpaceLeftInPage++;
    }

    // si el espacio libre no es suficiente retornar -1 (espacio debiese ser suficiente, encontrado por getPage)
    if( szOfFreeSpace < szRecord )
    {
        // liberar data
        free(data);

        // registro no pudo ser insertado
        return -22;
    }

    // actualizar tamaño y offset del registro
    curSlot->_szRecord = szRecord;
    curSlot->_offRecord = *ptrVarForFreeSpace;

    // actualizar comienzo del espacio libre
    *ptrVarForFreeSpace += szRecord;

    // asignar número de slots o una variable por referencia
    slotNum = (*ptrNumSlots) - 1;

    // asignar slot pasado por referencia
    slot = *curSlot;

    // escribir página de datos al archivo
    if( (errCode = fileHandle.writePage(pagenum, data)) != 0 )
    {
        // liberar página de datos
        free(data);

        return errCode; // retornar error
    }

    // liberar data
    free(data);

    return errCode; // operación exitosa
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    // 1. determinar tamaño de del registro
    // 2. obtener la página que contiene la cantidad necesaria de espacio libre
    // 3. obtener el lugar para el nuevo registro en la página de datos encontrada
    // 4. trabajo con la pagina de datos
    // 4.1 leer pagina (desde disco a memoria)
    // 4.2 copiar datos de registro en pagina
    // 4.3 escribir pagina (registrar cambios a disco)
    // 5. actualizar rid con num de pagina y num de slot

    RC errCode = 0;
    if( data == NULL )
    {
        return -11;
    }

    // determinar tamaño de del registro
    unsigned int recordSize = sizeOfRecord(recordDescriptor, data);
    // verificar que data no es más grande que el máximo permitido en una página
    if( recordSize >= MAX_SIZE_OF_RECORD )
    {
        return -21;	// registro excede el tamaño de página
    }
    unsigned int freeSpaceLeft = 0;
    // obtener la página que contiene la cantidad necesaria de espacio libre
    PageNum datapagenum = 0, headerpagenum = 0;
    if( (errCode = getDataPage(fileHandle, recordSize, datapagenum, headerpagenum, freeSpaceLeft)) != 0 )
    {
        return errCode; // retornar error desde la obtención de página
    }
    // obtener el lugar para el nuevo registro in la página de datos encontrada
    PageDirSlot pds;
    unsigned int slotNum = 0;
    if( (errCode = findRecordSlot(fileHandle, datapagenum, recordSize, pds, slotNum, freeSpaceLeft)) != 0 )
    {
        return errCode;
    }
    // leer data de la página de datos
    char* dataPage = (char*)malloc(PAGE_SIZE);
    memset(dataPage, 0, PAGE_SIZE);
    if( (errCode = fileHandle.readPage(datapagenum, dataPage)) != 0 )
    {
        return errCode;
    }
    // obtener puntero al registro
    char* ptrRecord = (char*)dataPage + pds._offRecord;
    // copiar data del registro
    memcpy(ptrRecord, data, recordSize);
    // escribir página al archivo
    if( (errCode = fileHandle.writePage(datapagenum, dataPage)) != 0 )
    {
        // liberar página de datos
        free(dataPage);
        return errCode; // retornar error
    }
    // liberar página de datos
    free(dataPage);

    // asignar rid
    rid.pageNum = datapagenum;
    rid.slotNum = slotNum;
    return 0; // operación exitosa
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    RC errCode = 0;

    if( data == NULL )
    {
        return -11; // data está corrompida
    }

    if( rid.pageNum == 0 || rid.pageNum >= fileHandle.getNumberOfPages() )
    {
        return -23; // rid no fue seteado correctamente
    }

    // alocar arreglo para guardar contenido de la página de datos
    void* dataPage = malloc(PAGE_SIZE);
    memset(dataPage, 0, PAGE_SIZE);

    // leer página de datos
    if( (errCode = fileHandle.readPage(rid.pageNum, dataPage)) != 0 )
    {
        // liberar dataPage
        free(dataPage);

        return errCode; // retornar error
    }

    /***
     * Página de datos tiene el siguiente formato:
     * [lista de registros sin espacio de separación][espacio libre para nuevos registros][directorio slots][número de slots:(unsigned int)][offset desde inicio de página al inicio del espacio libre:unsigned int]
     * ^                                                                                  ^                ^                                                                                                       ^
     * inicio de página                                                         inicio de dirSlot      fin de dirSlot                                                                                  fin de página
     ***/

    // obtener puntero al final del directorio de slots
    PageDirSlot* ptrEndOfDirSlot = (PageDirSlot*)((char*)dataPage + PAGE_SIZE - 2 * sizeof(unsigned int));

    // encontrar el número de slots en el directorio
    unsigned int numSlots = *((unsigned int*)ptrEndOfDirSlot);

    // verificar si rid es correcto en término de slots indexados
    if( rid.slotNum > numSlots )
    {
        // liberar página de datos
        free(dataPage);

        return -23; // rid no está seteado correctamente
    }

    // obtener slot
    PageDirSlot* curSlot = (PageDirSlot*)(ptrEndOfDirSlot - rid.slotNum - 1);

    // obtener offset del slot
    unsigned int offRecord = curSlot->_offRecord;
    unsigned int szRecord = curSlot->_szRecord;

    // verificar si atributos de slot son correctos
    if( offRecord == 0 && szRecord == 0 )
    {
        // liberar página de datos
        free(dataPage);
        return -24;	// error: directorio de slots tiene información incorrecta o registro fue eliminado
    }

    // determinar puntero al registro
    char* ptrRecord = (char*)(dataPage) + offRecord;

    // caso especial: en caso que registro sea TombStone
    if( szRecord == (unsigned int)-1 )
    {
        // redirigir a ubicación diferente; (page, slot) que es especificada en el cuerpo del registro
        RID* ptrNewRid = (RID*)ptrRecord;

        // intentar leer este registro
        return readRecord(fileHandle, recordDescriptor, *ptrNewRid, data);
    }

    // copiar contenido de registro
    memcpy(data, ptrRecord, szRecord);

    // liberar página de datos
    free(dataPage);

    return errCode; // operación exitosa
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    // declarar iterador para valores del descriptor
    vector<Attribute>::const_iterator i = recordDescriptor.begin();

    // puntero actual para iterar en data
    const void* ptr = data;

    // marcar inicio de registro con caracter "<"
    std::cout << "<";

    // iterar sobre los descriptores
    for(; i != recordDescriptor.end(); i++)
    {
        // insertar comas entre elementos
        if( ptr != data )
        {
            std::cout << ", ";
        }

        int ival = 0;
        float fval = 0.0f;
        char* carr = NULL;

        // dependiendo del tipo del elemento, realizar conversión e imprimir
        switch(i->type)
        {
            case TypeInt:
                // obtener int mediante casting del puntero a arreglo y utilizando el primer elemento
                ival = ((int*)ptr)[0];

                // imprimir valor y tipo
                std::cout << ival << ":Integer";

                // incrementar puntero ptr considerando que se leyó un int
                ptr = (void*)((char*)ptr + sizeof(int));

                break;
            case TypeReal:
                // obtener real (float) mediante casting del puntero a arreglo y utilizando el primer elemento
                fval = ((float*)ptr)[0];

                // imprimir valor y tipo
                std::cout << fval << ":Real";

                // incrementar puntero ptr considerando que se leyó un float
                ptr = (void*)((char*)ptr + sizeof(float));

                break;
            case TypeVarChar:
                // Varchar is más complejo que los casos anteriores. Está formado por tuplas <int, char*>:
                // primer elemento de la tupla es el tamaño del arreglo de caracteres, el segundo es el arreglo de caracteres

                // obtener el primer elemento, tamaño del arreglo de caracteres
                ival = ((int*)ptr)[0];

                // incrementar puntero PTR al inicio del segundo elemento
                ptr = (void*)((char*)ptr + sizeof(int));

                // crear un arreglo de caracteres
                carr = (char*)malloc(ival + 1);
                memset(carr, 0, ival + 1);

                // copiar contenido del segundo elemento en el arreglo
                memcpy((void*)carr, ptr, ival);

                // imprimir arreglo de caracteres y tipo
                std::cout << carr << ":VarChar";

                // incrementar puntero PTR  por iVal (tamaño del arreglo de caracteres)
                ptr = (void*)((char*)ptr + ival);

                // liberar arreglo de caracteres
                free(carr);

                break;
        }
    }

    // marcar find del registro con caracter ">"
    std::cout << ">" << std::endl;

    return 0; // operación exitosa
}

RC RecordBasedFileManager::deleteRecords(FileHandle &fileHandle)
{
    /*
     * AGREGUE SU CODIGO AQUI
     
     */

    return -1;
}

RC RecordBasedFileManager::deleteRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid)
{
    /*
     * AGREGUE SU CODIGO AQUI
     */

    return -1;
}

/***
 * Determinar tamaño de registro en bytes
***/
unsigned int sizeOfRecord(const vector<Attribute> &recordDescriptor, const void *data)
{
    unsigned int size = 0, szOfCharArr = 0;

    // iterar a través de los descriptores de registros e identificar el tamaño de cada campo
    vector<Attribute>::const_iterator i = recordDescriptor.begin();
    for(; i != recordDescriptor.end(); i++)
    {
        switch( i->type )
        {
            case TypeInt:
                // sumar tamaño de campo entero
                size += sizeof(int);

                // incrementar al siguiente campo
                data = (void*)((char*)data + sizeof(int));
                break;
            case TypeReal:
                // sumar tamaño de campo real
                size += sizeof(float);

                // incrementar al siguiente campo
                data = (void*)((char*)data + sizeof(float));
                break;
            case TypeVarChar:
                // obtener entero que representa el tamaño de arreglo de caracteres y sumarlo
                szOfCharArr = ((unsigned int*)(data))[0];
                size += sizeof(int) + szOfCharArr;

                // incrementar al siguiente campo
                data = (void*)((char*)data + sizeof(int) + szOfCharArr);
                break;
        }
    }

    //return size
    return size;
}
