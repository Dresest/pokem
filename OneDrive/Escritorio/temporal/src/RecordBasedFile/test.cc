#include <iostream>
#include <string>
#include <cassert>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <stdio.h>
#include "PagedFileManager.h"
#include "RecordBasedFileManager.h"


using namespace std;
const int success = 0;

// Verificar si achivo existe
bool FileExists(string fileName)
{
    struct stat stFileInfo;
    if(stat(fileName.c_str(), &stFileInfo) == 0) return true;
    else return false;
}

// Función para preparar datos para ser insertados/leidos
void prepareRecord(const int nameLength, const string &name, const int age, const float height, const int salary, void *buffer, int *recordSize)
{
    int offset = 0;

    memcpy((char *)buffer + offset, &nameLength, sizeof(int));
    offset += sizeof(int);
    memcpy((char *)buffer + offset, name.c_str(), nameLength);
    offset += nameLength;

    memcpy((char *)buffer + offset, &age, sizeof(int));
    offset += sizeof(int);

    memcpy((char *)buffer + offset, &height, sizeof(float));
    offset += sizeof(float);

    memcpy((char *)buffer + offset, &salary, sizeof(int));
    offset += sizeof(int);

    *recordSize = offset;
}
void prepareMyRecord(const int key, const int valLength, char c, void *buffer, int *recordSize)
{
    int offset = 0;

    string val;
    for(int i = 0; i < valLength; i++)
        val.push_back(c);

    memcpy((char *)buffer + offset, &valLength, sizeof(int));
    offset += sizeof(int);
    memcpy((char *)buffer + offset, val.c_str(), valLength);
    offset += valLength;

    memcpy((char *)buffer + offset, &key, sizeof(int));
    offset += sizeof(int);

    *recordSize = offset;
}

void prepareLargeRecord(const int index, void *buffer, int *size)
{
    int offset = 0;

    // calcular contador
    int count = index % 50 + 1;
    // calcular letra
    char text = index % 26 + 97;
    for(int i = 0; i < 10; i++)
    {
        memcpy((char *)buffer + offset, &count, sizeof(int));
        offset += sizeof(int);
        for(int j = 0; j < count; j++)
        {
            memcpy((char *)buffer + offset, &text, 1);
            offset += 1;
        }

        // calcular número int
        memcpy((char *)buffer + offset, &index, sizeof(int));
        offset += sizeof(int);

        // calcular número float
        float real = (float)(index + 1);
        memcpy((char *)buffer + offset, &real, sizeof(float));
        offset += sizeof(float);
    }
    *size = offset;
}
void createRecordDescriptor(vector<Attribute> &recordDescriptor) {
    Attribute attr;
    attr.name = "EmpName";
    attr.type = TypeVarChar;
    attr.length = (AttrLength)30;
    recordDescriptor.push_back(attr);
    attr.name = "Age";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);
    attr.name = "Height";
    attr.type = TypeReal;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);
    attr.name = "Salary";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);
}

void createLargeRecordDescriptor(vector<Attribute> &recordDescriptor)
{
    int index = 0;
    char *suffix = (char *)malloc(10);
    for(int i = 0; i < 10; i++)
    {
        Attribute attr;
        sprintf(suffix, "%d", index);
        attr.name = "attr";
        attr.name += suffix;
        attr.type = TypeVarChar;
        attr.length = (AttrLength)50;
        recordDescriptor.push_back(attr);
        index++;
        sprintf(suffix, "%d", index);
        attr.name = "attr";
        attr.name += suffix;
        attr.type = TypeInt;
        attr.length = (AttrLength)4;
        recordDescriptor.push_back(attr);
        index++;
        sprintf(suffix, "%d", index);
        attr.name = "attr";
        attr.name += suffix;
        attr.type = TypeReal;
        attr.length = (AttrLength)4;
        recordDescriptor.push_back(attr);
        index++;
    }
    free(suffix);
}

void createMyRecordDescriptor(vector<Attribute> &recordDescriptor)
{
    Attribute attr;
    attr.name = "key";
    attr.type = TypeVarChar;
    attr.length = (AttrLength)4000;
    recordDescriptor.push_back(attr);
    attr.name = "val";
    attr.type = TypeInt;
    attr.length = (AttrLength)4;
    recordDescriptor.push_back(attr);
}

int RBFTest_1(PagedFileManager *pfm)
{
    // Funciones a testear:
    // 1. Crear archivo
    cout << "****RBF Test Case 1****" << endl;
    RC rc;
    string fileName = "test";
    // Crear archivo "test"
    rc = pfm->createFile(fileName.c_str());
    assert(rc == success);
    if(FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been created." << endl << endl;
    }
    else
    {
        cout << "Failed to create file!" << endl;
        cout << "Test Case 1 Failed!" << endl << endl;
        return -1;
    }
    // Crear "test" nuevamente, debería fallar
    rc = pfm->createFile(fileName.c_str());
    assert(rc != success);
    cout << "Test Case 1 Passed!" << endl << endl;
    return 0;
}

int RBFTest_2(PagedFileManager *pfm)
{
    // Funciones a testear
    // 1. Eliminar archivo
    cout << "****In RBF Test Case 2****" << endl;
    RC rc;
    string fileName = "test";
    rc = pfm->destroyFile(fileName.c_str());
    assert(rc == success);
    if(!FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been destroyed." << endl << endl;
        cout << "Test Case 2 Passed!" << endl << endl;
        return 0;
    }
    else
    {
        cout << "Failed to destroy file!" << endl;
        cout << "Test Case 2 Failed!" << endl << endl;
        return -1;
    }
}

int RBFTest_3(PagedFileManager *pfm)
{
    // Funciones a testear:
    // 1. Crear archivo
    // 2. Abrir archivo
    // 3. Obtener número de páginas
    // 4. Cerrar archivo
    cout << "****In RBF Test Case 3****" << endl;
    RC rc;
    string fileName = "test_1";
    // Crear archivo "test_1"
    rc = pfm->createFile(fileName.c_str());
    assert(rc == success);
    if(FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been created." << endl;
    }
    else
    {
        cout << "Failed to create file!" << endl;
        cout << "Test Case 3 Failed!" << endl << endl;
        return -1;
    }
    // Abrir archivo "test_1"
    FileHandle fileHandle;
    rc = pfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);
    // Obtener el número de páginas
    unsigned count = fileHandle.getNumberOfPages();
    assert(count == (unsigned)0);
    // Cerrar archivo "test_1"
    rc = pfm->closeFile(fileHandle);
    assert(rc == success);
    cout << "Test Case 3 Passed!" << endl << endl;
    return 0;
}

int RBFTest_4(PagedFileManager *pfm)
{
    // Funciones a testear:
    // 1. Abrir archivo
    // 2. Añadir página
    // 3. Obtener número de páginas
    // 3. Cerrar archivo
    cout << "****In RBF Test Case 4****" << endl;
    RC rc;
    string fileName = "test_1";
    // Abrir archivo "test_1"
    FileHandle fileHandle;
    rc = pfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);
    // Añadir la primera página
    void *data = malloc(PAGE_SIZE);
    for(unsigned i = 0; i < PAGE_SIZE; i++)
    {
        *((char *)data+i) = i % 94 + 32;
    }
    rc = fileHandle.appendPage(data);
    assert(rc == success);

    // Obtener número de páginas
    unsigned count = fileHandle.getNumberOfPages();
    assert(count == (unsigned)1);
    // Cerrar archivo "test_1"
    rc = pfm->closeFile(fileHandle);
    assert(rc == success);
    free(data);
    cout << "Test Case 4 Passed!" << endl << endl;
    return 0;
}

int RBFTest_5(PagedFileManager *pfm)
{
    // Funciones a testear:
    // 1. Abrir archivo
    // 2. Leer página
    // 3. Cerrar archivo
    cout << "****In RBF Test Case 5****" << endl;
    RC rc;
    string fileName = "test_1";
    // Abrir archivo "test_1"
    FileHandle fileHandle;
    rc = pfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);
    // Leer la primera página
    void *buffer = malloc(PAGE_SIZE);
    rc = fileHandle.readPage(0, buffer);
    assert(rc == success);

    // Verificar integridad de la página
    void *data = malloc(PAGE_SIZE);
    for(unsigned i = 0; i < PAGE_SIZE; i++)
    {
        *((char *)data+i) = i % 94 + 32;
    }
    rc = memcmp(data, buffer, PAGE_SIZE);
    assert(rc == success);

    // Cerrar archivo "test_1"
    rc = pfm->closeFile(fileHandle);
    assert(rc == success);
    free(data);
    free(buffer);
    cout << "Test Case 5 Passed!" << endl << endl;
    return 0;
}

int RBFTest_6(PagedFileManager *pfm)
{
    // Funciones a testear
    // 1. Abrir archivo
    // 2. Escribir página
    // 3. Leer página
    // 4. Cerrar archivo
    // 5. Eliminar archivo
    cout << "****In RBF Test Case 6****" << endl;
    RC rc;
    string fileName = "test_1";
    // Open the file "test_1"
    FileHandle fileHandle;
    rc = pfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);
    // Actualizar primera página
    void *data = malloc(PAGE_SIZE);
    for(unsigned i = 0; i < PAGE_SIZE; i++)
    {
        *((char *)data+i) = i % 10 + 32;
    }
    rc = fileHandle.writePage(0, data);
    assert(rc == success);
    // Leer página
    void *buffer = malloc(PAGE_SIZE);
    rc = fileHandle.readPage(0, buffer);
    assert(rc == success);
    // Verificar integridad
    rc = memcmp(data, buffer, PAGE_SIZE);
    assert(rc == success);

    // Cerrar archivo "test_1"
    rc = pfm->closeFile(fileHandle);
    assert(rc == success);
    free(data);
    free(buffer);
    // Destruir archivo
    rc = pfm->destroyFile(fileName.c_str());
    assert(rc == success);

    if(!FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been destroyed." << endl;
        cout << "Test Case 6 Passed!" << endl << endl;
        return 0;
    }
    else
    {
        cout << "Failed to destroy file!" << endl;
        cout << "Test Case 6 Failed!" << endl << endl;
        return -1;
    }
}

int RBFTest_7(PagedFileManager *pfm)
{
    // Funciones a testear:
    // 1. Crear archivo
    // 2. Abrir archivo
    // 3. Añadir página
    // 4. Obtener número de páginas
    // 5. Leer página
    // 6. Escribir página
    // 7. Cerrar archivo
    // 8. Destruir archivo
    cout << "****In RBF Test Case 7****" << endl;
    RC rc;
    string fileName = "test_2";
    // crear archivo "test_2"
    rc = pfm->createFile(fileName.c_str());
    assert(rc == success);
    if(FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been created." << endl;
    }
    else
    {
        cout << "Failed to create file!" << endl;
        cout << "Test Case 7 Failed!" << endl << endl;
        return -1;
    }
    // abrir archivo "test_2"
    FileHandle fileHandle;
    rc = pfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);
    // añadir 50 páginas
    void *data = malloc(PAGE_SIZE);
    for(unsigned j = 0; j < 50; j++)
    {
        for(unsigned i = 0; i < PAGE_SIZE; i++)
        {
            *((char *)data+i) = i % (j+1) + 32;
        }
        rc = fileHandle.appendPage(data);
        assert(rc == success);
    }
    cout << "50 Pages have been successfully appended!" << endl;

    // obtener el número de páginas
    unsigned count = fileHandle.getNumberOfPages();
    assert(count == (unsigned)50);
    // leer página 25 y verificar integridad
    void *buffer = malloc(PAGE_SIZE);
    rc = fileHandle.readPage(24, buffer);
    assert(rc == success);
    for(unsigned i = 0; i < PAGE_SIZE; i++)
    {
        *((char *)data + i) = i % 25 + 32;
    }
    rc = memcmp(buffer, data, PAGE_SIZE);
    assert(rc == success);
    cout << "The data in 25th page is correct!" << endl;
    // actualizar página 25
    for(unsigned i = 0; i < PAGE_SIZE; i++)
    {
        *((char *)data+i) = i % 60 + 32;
    }
    rc = fileHandle.writePage(24, data);
    assert(rc == success);
    // leer la página 25 y verificar integridad
    rc = fileHandle.readPage(24, buffer);
    assert(rc == success);

    rc = memcmp(buffer, data, PAGE_SIZE);
    assert(rc == success);
    // cerrar archivo "test_2"
    rc = pfm->closeFile(fileHandle);
    assert(rc == success);
    // eliminar archivo
    rc = pfm->destroyFile(fileName.c_str());
    assert(rc == success);
    free(data);
    free(buffer);
    if(!FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been destroyed." << endl;
        cout << "Test Case 7 Passed!" << endl << endl;
        return 0;
    }
    else
    {
        cout << "Failed to destroy file!" << endl;
        cout << "Test Case 7 Failed!" << endl << endl;
        return -1;
    }
}

int RFBTest_boundary_cases(PagedFileManager *pfm)
{
    // setup
    remove("new_file");
    cout << "****In RBF Test Case Special****" << endl;
    RC rc;
    // 1. test de frontera - creat archivo con nombre  ""
    rc = pfm->createFile("");
    assert(rc != success);
    // 2. test de frontera - eliminar archivo no existente
    rc = pfm->destroyFile("non_existing_file");
    assert(rc != success);
    // 3. eliminar archivo que ya fue abierto
    rc = pfm->createFile("new_file");
    assert(rc == success);
    FileHandle fileHandle;
    rc = pfm->openFile("new_file", fileHandle);
    assert(rc == success);
    rc = pfm->destroyFile("new_file");
    assert(rc != success);
    // 4. verificar que luego de eliminación infructuosa, el archivo puede ser escrito y leído
    void *data = malloc(PAGE_SIZE);
    for(unsigned i = 0; i < PAGE_SIZE; i++)
    {
        *((char *)data+i) = i % 10 + 32;
    }
    // escribir página
    rc = fileHandle.appendPage(data);
    assert(rc == success);
    // leer página
    void *buffer = malloc(PAGE_SIZE);
    rc = fileHandle.readPage(0, buffer);
    assert(rc == success);
    // verificar integridad
    rc = memcmp(data, buffer, PAGE_SIZE);
    assert(rc == success);
    // 5. abrir un archivo con un manejador usado
    rc = pfm->openFile("new_file", fileHandle);
    assert(rc != success);
    // 6. leer más allá del fin de archivo
    rc = fileHandle.readPage(1, buffer);
    assert(rc != success);
    // 7. escribir más allá del fin de archivo
    rc = fileHandle.writePage(1, data);
    assert(rc != success);
    // 8. leer desde data con terminación Null
    free(data);
    data = NULL;
    rc = fileHandle.readPage(0, data);
    assert(rc != success);
    // 9. escribir un data con terminación Null
    rc = fileHandle.writePage(0, data);
    assert(rc != success);
    // 10. intentar cerrar archivo dos veces
    rc = pfm->closeFile(fileHandle);
    assert(rc == success);
    rc = pfm->closeFile(fileHandle);
    assert(rc != success);
    // 11. añadir data a un archivo no abierto
    rc = fileHandle.appendPage(data);
    assert(rc != success);
    // 12. leer desde un archivo no abierto
    rc = fileHandle.readPage(0, data);
    assert(rc != success);
    // 13. obtener el número de páginas usando el manejador de archivos y cerrar el archivo
    int numPages = fileHandle.getNumberOfPages();
    assert(numPages < 0);
    // 14. crear un manejador de archivo vacio e intentar cerrarlo con archivo asociado
    FileHandle emptyHandle;
    rc = pfm->closeFile(emptyHandle);
    assert(rc != success);
    cout << "Test Case Special Passed!" << endl << endl;
    return 0;
}

int RBFTest_8(RecordBasedFileManager *rbfm) {
    // Funciones a testear
    // 1. Crear archivo con estructura de registros
    // 2. Abrir archivo
    // 3. Insertar registro
    // 4. Leer registro
    // 5. Cerrar archivo
    // 6. Eliminar archivo
    cout << "****In RBF Test Case 8****" << endl;

    RC rc;
    string fileName = "test_3";
    // Crear archivo "test_3"
    rc = rbfm->createFile(fileName.c_str());
    assert(rc == success);
    if(FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been created." << endl;
    }
    else
    {
        cout << "Failed to create file!" << endl;
        cout << "Test Case 8 Failed!" << endl << endl;
        return -1;
    }
    // Abrir archivo "test_3"
    FileHandle fileHandle;
    rc = rbfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);


    RID rid;
    int recordSize = 0;
    void *record = malloc(100);
    void *returnedData = malloc(100);
    vector<Attribute> recordDescriptor;
    createRecordDescriptor(recordDescriptor);

    // Insertar un nuevo registro a archivo
    prepareRecord(6, "Peters", 24, 170.1, 5000, record, &recordSize);
    cout << "Insert Data:" << endl;
    //rbfm->printRecord(recordDescriptor, record);

    rc = rbfm->insertRecord(fileHandle, recordDescriptor, record, rid);
    assert(rc == success);

    // Dado el rid, leer el registro desde archivo
    rc = rbfm->readRecord(fileHandle, recordDescriptor, rid, returnedData);
    assert(rc == success);
    cout << "Returned Data:" << endl;
    //rbfm->printRecord(recordDescriptor, returnedData);
    // Comparar si ambos bloques de memoria son iguales
    if(memcmp(record, returnedData, recordSize) != 0)
    {
        cout << "Test Case 8 Failed!" << endl << endl;
        free(record);
        free(returnedData);
        return -1;
    }

    // Cerrar archivo "test_3"
    rc = rbfm->closeFile(fileHandle);
    assert(rc == success);
    // Eliminar archivo
    rc = rbfm->destroyFile(fileName.c_str());
    assert(rc == success);

    free(record);
    free(returnedData);
    cout << "Test Case 8 Passed!" << endl << endl;

    return 0;
}

int RBFTest_9(RecordBasedFileManager *rbfm, vector<RID> &rids, vector<int> &sizes) {
    // Funciones a testear
    // 1. Crear archivo con estructura de registros
    // 2. Abrir archivo
    // 3. Insertar múltiples registros
    // 4. Cerrar archivo
    cout << "****In RBF Test Case 9****" << endl;

    RC rc;
    string fileName = "test_4";
    // Crear archivo "test_4"
    rc = rbfm->createFile(fileName.c_str());
    assert(rc == success);
    if(FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been created." << endl;
    }
    else
    {
        cout << "Failed to create file!" << endl;
        cout << "Test Case 9 Failed!" << endl << endl;
        return -1;
    }
    // Abrir archivo "test_4"
    FileHandle fileHandle;
    rc = rbfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);
    RID rid;
    void *record = malloc(1000);
    int numRecords = 2000;
    void *returnedData = malloc(1000);	//
    vector<Attribute> recordDescriptor;
    createLargeRecordDescriptor(recordDescriptor);
    for(unsigned i = 0; i < recordDescriptor.size(); i++)
    {
        // Quitar comentarios si desea revisar registros
        // cout << "Attribute Name: " << recordDescriptor[i].name << endl;
        // cout << "Attribute Type: " << (AttrType)recordDescriptor[i].type << endl;
        // cout << "Attribute Length: " << recordDescriptor[i].length << endl << endl;
    }
    // Insertar 2000 registros en archivo
    for(int i = 0; i < numRecords; i++)
    {
        // Testear insertar registro
        int size = 0;
        memset(record, 0, 1000);
        prepareLargeRecord(i, record, &size);
        rc = rbfm->insertRecord(fileHandle, recordDescriptor, record, rid);
        assert(rc == success);
        rids.push_back(rid);
        sizes.push_back(size);
        memset(returnedData, 0, 1000);	//
        rc = rbfm->readRecord(fileHandle, recordDescriptor, rid, returnedData);	//
        // rbfm->printRecord(recordDescriptor, record);	//
        // rbfm->printRecord(recordDescriptor, returnedData);	//
        if(memcmp(returnedData, record, sizes[i]) != 0)	//
        {	//
            cout << "not equal";	//
        }	//
    }
    // Cerrar archivo "test_4"
    rc = rbfm->closeFile(fileHandle);
    assert(rc == success);
    free(record);
    cout << "Test Case 9 Passed!" << endl << endl;
    return 0;
}

int RBFTest_10(RecordBasedFileManager *rbfm, vector<RID> &rids, vector<int> &sizes) {
    // Funciones a testear
    // 1. Abrir archivo con estructura de registros
    // 2. Leer múltiples registros
    // 3. Cerrar archivo
    // 4. Eliminar archivo
    cout << "****In RBF Test Case 10****" << endl;

    RC rc;
    string fileName = "test_4";
    // Abrir archivo "test_4"
    FileHandle fileHandle;
    rc = rbfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);

    int numRecords = 2000;
    void *record = malloc(1000);
    void *returnedData = malloc(1000);
    vector<Attribute> recordDescriptor;
    createLargeRecordDescriptor(recordDescriptor);

    for(int i = 0; i < numRecords; i++)
    {
        memset(record, 0, 1000);
        memset(returnedData, 0, 1000);
        rc = rbfm->readRecord(fileHandle, recordDescriptor, rids[i], returnedData);
        assert(rc == success);

        // cout << "Returned Data:" << endl;
        // rbfm->printRecord(recordDescriptor, returnedData);
        int size = 0;
        prepareLargeRecord(i, record, &size);
        // rbfm->printRecord(recordDescriptor, record);
        if(memcmp(returnedData, record, sizes[i]) != 0)
        {
            cout << "Test Case 10 Failed!" << endl << endl;
            free(record);
            free(returnedData);
            return -1;
        }
    }

    // Cerrar archivo "test_4"
    rc = rbfm->closeFile(fileHandle);
    assert(rc == success);

    rc = rbfm->destroyFile(fileName.c_str());
    assert(rc == success);
    if(!FileExists(fileName.c_str())) {
        cout << "File " << fileName << " has been destroyed." << endl << endl;
        free(record);
        free(returnedData);
        cout << "Test Case 10 Passed!" << endl << endl;
        return 0;
    }
    else {
        cout << "Failed to destroy file!" << endl;
        cout << "Test Case 10 Failed!" << endl << endl;
        free(record);
        free(returnedData);
        return -1;
    }
}

int RBFTest_11(RecordBasedFileManager *rbfm) {
    // Funciones a testear
    // 1. Crear archivo con estructura de registros
    // 2. Abrir archivo
    // 3. Insertar registro
    // 4. Leer registro
    // 5. Eliminar registro
    // 6. Cerrar archivo
    // 7. Eliminar archivo
    cout << "****In RBF Test Case 11****" << endl;

    RC rc;
    string fileName = "test_11";
    // Crear archivo "test_11"
    rc = rbfm->createFile(fileName.c_str());
    assert(rc == success);
    if(FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been created." << endl;
    }
    else
    {
        cout << "Failed to create file!" << endl;
        cout << "Test Case 8 Failed!" << endl << endl;
        return -1;
    }
    // Abrir archivo "test_11"
    FileHandle fileHandle;
    rc = rbfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);


    RID rid;
    int recordSize = 0;
    void *record = malloc(100);
    void *returnedData = malloc(100);
    vector<Attribute> recordDescriptor;
    createRecordDescriptor(recordDescriptor);

    // Insertar un nuevo registro a archivo
    prepareRecord(6, "Peters", 24, 170.1, 5000, record, &recordSize);
    cout << "Insert Data:" << endl;
    rbfm->printRecord(recordDescriptor, record);

    rc = rbfm->insertRecord(fileHandle, recordDescriptor, record, rid);
    assert(rc == success);

    // Dado el rid, leer el registro desde archivo
    rc = rbfm->readRecord(fileHandle, recordDescriptor, rid, returnedData);
    assert(rc == success);
    cout << "Returned Data:" << endl;
    rbfm->printRecord(recordDescriptor, returnedData);

    // Eliminar registro
    rc = rbfm->deleteRecord(fileHandle, recordDescriptor, rid);
    assert(rc == success);

    // Dado el rid, leer el registro desde archivo
    void *returnedDeletedData = malloc(100);
    rc = rbfm->readRecord(fileHandle, recordDescriptor, rid, returnedDeletedData);
    // Verificar que registro se haya eliminado correctamente
    if (memcmp(record, returnedDeletedData, recordSize) == 0) {
        cout << "Test Case 11 Failed!" << endl << endl;
        free(record);
        free(returnedData);
        free(returnedDeletedData);
        rbfm->closeFile(fileHandle);
        rbfm->destroyFile(fileName.c_str());
        return -1;
    }

    // Cerrar archivo "test_11"
    rc = rbfm->closeFile(fileHandle);
    assert(rc == success);
    // Eliminar archivo
    rc = rbfm->destroyFile(fileName.c_str());
    assert(rc == success);

    free(record);
    free(returnedData);
    free(returnedDeletedData);
    cout << "Test Case 11  Passed!" << endl << endl;

    return 0;
}

int RBFTest_12(RecordBasedFileManager *rbfm, vector<RID> &rids, vector<int> &sizes)
{
    // Funciones a testear
    // 1. Crear archivo con estructura de registros
    // 2. Abrir archivo
    // 3. Insertar multiples registros
    // 4. Leer multiples registros
    // 5. Eliminar multiples registros
    // 6. Lee un registro para verificar borrado de registros
    // 7. Cerrar archivo
    // 8. Eliminar archivo
    cout << "****In RBF Test Case 12****" << endl;

    RC rc;
    string fileName = "test_12";
    // Crear archivo "test_12"
    rc = rbfm->createFile(fileName.c_str());
    assert(rc == success);
    if (FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been created." << endl;
    }
    else
    {
        cout << "Failed to create file!" << endl;
        cout << "Test Case 12 Failed!" << endl << endl;
        return -1;
    }
    // Abrir archivo "test_12"
    FileHandle fileHandle;
    rc = rbfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);
    RID rid;
    void *record = malloc(1000);
    int numRecords = 2000;
    void *returnedData = malloc(1000);	
    vector<Attribute> recordDescriptor;
    createLargeRecordDescriptor(recordDescriptor);
    for (unsigned i = 0; i < recordDescriptor.size(); i++)
    {
        // Quitar comentarios si desea revisar registros
        // cout << "Attribute Name: " << recordDescriptor[i].name << endl;
        // cout << "Attribute Type: " << (AttrType)recordDescriptor[i].type << endl;
        // cout << "Attribute Length: " << recordDescriptor[i].length << endl << endl;
    }
    // Insertar 2000 registros en archivo
    for (int i = 0; i < numRecords; i++)
    {
        // Testear insertar registro
        int size = 0;
        memset(record, 0, 1000);
        prepareLargeRecord(i, record, &size);
        rc = rbfm->insertRecord(fileHandle, recordDescriptor, record, rid);
        assert(rc == success);
        rids.push_back(rid);
        sizes.push_back(size);
        memset(returnedData, 0, 1000);	//
        rc = rbfm->readRecord(fileHandle, recordDescriptor, rid, returnedData);	//
        // cout << "first" << endl;
        // rbfm->printRecord(recordDescriptor, record);
        // cout << "second" << endl;
        // rbfm->printRecord(recordDescriptor, returnedData);
        if(memcmp(returnedData, record, sizes[i]) != 0)	//
        {	//
            cout << "not equal";	//
        }	//
    }

    //Leer multiples registros
    for(int i = 0; i < numRecords; i++)
    {
        memset(record, 0, 1000);
        memset(returnedData, 0, 1000);
        rc = rbfm->readRecord(fileHandle, recordDescriptor, rids[i], returnedData);
        assert(rc == success);

        //cout << "Returned Data:" << endl;
        //rbfm->printRecord(recordDescriptor, returnedData);
        int size = 0;
        prepareLargeRecord(i, record, &size);
        // rbfm->printRecord(recordDescriptor, record);
        if(memcmp(returnedData, record, sizes[i]) != 0)
        {
            cout << "Test Case 12 Failed!" << endl << endl;
            free(record);
            free(returnedData);
            return -1;
        }
    }

    //Elimina los registros
    if((rc = rbfm->deleteRecords(fileHandle)) != 0)
    {
        cout << "Test Case 12 Failed!" << endl << endl;
        free(record);
        free(returnedData);
        return -1;
    }

    //Verifica que se hayan eliminado mediante la lectura del primer registro ingresado
    if((rc = rbfm->readRecord(fileHandle, recordDescriptor, rids[0], returnedData)) != -24)
    {
        cout << "Test Case 12 Failed!" << endl << endl;
        free(record);
        free(returnedData);
        return -1;
    }
    
    // Cerrar archivo "test_12"
    rc = rbfm->closeFile(fileHandle);
    assert(rc == success);
    // Eliminar archivo
    rc = rbfm->destroyFile(fileName.c_str());
    assert(rc == success);

    free(record);
    free(returnedData);
    cout << "Test Case 12 Passed!" << endl << endl;

    return 0;
}

int RBFTest_13(RecordBasedFileManager *rbfm, vector<RID> &rids, vector<int> &sizes)
{
    // Funciones a testear
    // 1. Crear archivo con estructura de registros
    // 2. Abrir archivo
    // 3. Insertar multiples registros
    // 4. Elimina la primera mitad de los registros insertados
    // 5. Verifica borrado de  registros.
    // 6. Cerrar archivo
    // 7. Eliminar archivo
    cout << "****In RBF Test Case 13****" << endl;

    RC rc;
    string fileName = "test_13";
    // Crear archivo "test_13"
    rc = rbfm->createFile(fileName.c_str());
    assert(rc == success);
    if(FileExists(fileName.c_str()))
    {
        cout << "File " << fileName << " has been created." << endl;
    }
    else
    {
        cout << "Failed to create file!" << endl;
        cout << "Test Case 13 Failed!" << endl << endl;
        return -1;
    }
    // Abrir archivo "test_13"
    FileHandle fileHandle;
    rc = rbfm->openFile(fileName.c_str(), fileHandle);
    assert(rc == success);
    RID rid;
    void *record = malloc(1000);
    int numRecords = 2000;
    void *returnedData = malloc(1000);	
    vector<Attribute> recordDescriptor;
    createLargeRecordDescriptor(recordDescriptor);
    for(unsigned i = 0; i < recordDescriptor.size(); i++)
    {
        // Quitar comentarios si desea revisar registros
        // cout << "Attribute Name: " << recordDescriptor[i].name << endl;
        // cout << "Attribute Type: " << (AttrType)recordDescriptor[i].type << endl;
        // cout << "Attribute Length: " << recordDescriptor[i].length << endl << endl;
    }
    // Insertar 2000 registros en archivo
    for(int i = 0; i < numRecords; i++)
    {
        // Testear insertar registro
        int size = 0;
        memset(record, 0, 1000);
        prepareLargeRecord(i, record, &size);
        rc = rbfm->insertRecord(fileHandle, recordDescriptor, record, rid);
        assert(rc == success);
        rids.push_back(rid);
        sizes.push_back(size);
        memset(returnedData, 0, 1000);	//
        rc = rbfm->readRecord(fileHandle, recordDescriptor, rid, returnedData);	//
        // rbfm->printRecord(recordDescriptor, record);	//
        // rbfm->printRecord(recordDescriptor, returnedData);	//
        if(memcmp(returnedData, record, sizes[i]) != 0)	//
        {
            cout << "not equal";
        }
    }

    // Elimina la primera mitad de los registros
    for(int i = 0; i < numRecords/2; i++)
    {
        rc = rbfm->deleteRecord(fileHandle, recordDescriptor, rids[i]);
    }

    // Verifica mediante la lectura de registros aleatorios dentro del rango de eliminacion
    for(int i = 0; i<100; i++)
    {
        int random = rand() % 1000;
        if((rc = rbfm->readRecord(fileHandle, recordDescriptor, rids[0], returnedData)) != -24)
        {
            cout << "Test Case 13 Failed!" << endl << endl;
            free(record);
            free(returnedData);
            return -1;
        }
    }

    // Cerrar archivo "test_13"
    rc = rbfm->closeFile(fileHandle);
    assert(rc == success);
    // Eliminar archivo
    rc = rbfm->destroyFile(fileName.c_str());
    assert(rc == success);

    free(record);
    free(returnedData);
    cout << "Test Case 13 Passed!" << endl << endl;

    return 0;
}

int main()
{
    // Obtener instancias de PagedFileManager y RecordBasedFileManager
    PagedFileManager *pfm = PagedFileManager::instance();
    RecordBasedFileManager *rbfm = RecordBasedFileManager::instance();


    // Eliminar archivos que pueden haber sido creados anteriormente
    remove("test");
    remove("test_1");
    remove("test_2");
    remove("test_3");
    remove("test_4");
    remove("test_my_1");
    remove("test_11");
    remove("test_12");

    /**
     * Tests base
     **/

    RBFTest_1(pfm);
    RBFTest_2(pfm);
    RBFTest_3(pfm);
    RBFTest_4(pfm);
    RBFTest_5(pfm);
    RBFTest_6(pfm);
    RBFTest_7(pfm);
    RFBTest_boundary_cases(pfm);
    RBFTest_8(rbfm);

    /**
     * Tests adicionales
     **/


    vector<RID> rids;
    vector<int> sizes;
    RBFTest_9(rbfm, rids, sizes);
    RBFTest_10(rbfm, rids, sizes);

    rids.clear();
    sizes.clear();

    /**
     * Tests de eliminación de registros
     **/

    RBFTest_11(rbfm);
    RBFTest_12(rbfm, rids, sizes);
    RBFTest_13(rbfm, rids, sizes);


    return 0;
}
