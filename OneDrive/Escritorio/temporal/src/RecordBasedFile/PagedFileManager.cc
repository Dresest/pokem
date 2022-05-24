#include "PagedFileManager.h"
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

/***
 * Códigos de error retornados por PagedFileManager:
 * -1 = archivo que se intenta crear ya existe
 * -2 = falla de fopen al creat/abrir nuevo archivo
 * -3 = conflicto en información del registro; entrada ya exisite
 * -4 = archivo no existe
 * -5 = archivo abierto no puede ser borrado/re-abierto
 * -6 = fallo al intentar eliminar un archivo
 * -7 = registro de archivo no existe en _files
 * -8 = manejador de archivo que es usado para un archivo está tratando de ser usado en otro
 * -9 = manejador de archivo no tiene archivo asociado
 * -10 = número de página accesado es mayor que el máximo disponible
 * -11 = data está corrompida
 * -12 = falla de fseek
 * -13 = falla de fread/fwrite
 * --- casos adicionales:
 * -14 = fileName es ilegal (NULL o empty string vacío)
 * -15 = número de páginas desconocido, contador de páginas no válido
***/

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
    {
        _pf_manager = new PagedFileManager();
        _pf_manager->_files = std::map<std::string, FileInfo>();
    }

    return _pf_manager;
}


PagedFileManager::PagedFileManager()
{
}


PagedFileManager::~PagedFileManager()
{
}

/***
 * Verifica si archivo existe
**/
bool PagedFileManager::isExisting(const char *fileName) const
{
    struct stat stFileInfo;
    if(stat(fileName, &stFileInfo) == 0) return true;
    else return false;
}

/***
 * Crea nuevo archivo
**/
RC PagedFileManager::createFile(const char *fileName)
{
    // verificar que nombre sea válido
    if( fileName == NULL || strlen(fileName) == 0 )
    {
        return -14;
    }

    // verificar si archivo existe
    if(isExisting(fileName))
    {
        return -1;	// error :archivo con el mismo nombre ya exisite
    }

    // crear nuevo archivo
    FILE *write_ptr = fopen(fileName,"wb");

    // verificar que el archivo se creó correctamente
    if(!write_ptr)
    {
        return -2;	// error: sistema no es capáz de crear nuevo archivo
    }

    // crear string para el nombre de archivo
    std::string name = std::string(fileName);

    // crear información de archivo con estructura FileInfocreate
    FileInfo info = FileInfo(name, 0, 0);

    // registrar nuevo archivo en hash map (_files)
    if( _files.insert( std::pair<std::string, FileInfo>(name, info) ).second == false )
        return -3;	// archivo con referencia a FILE* ya existe

    // cerrar archivo
    fclose(write_ptr);
    return 0; // archivo creado exitosamente
}


RC PagedFileManager::destroyFile(const char *fileName)
{
    // verificar nombres de archivos ilegales
    if( fileName == NULL || strlen(fileName) == 0 )
    {
        return -14;
    }

    // Iterador que referencia a el item en _files
    std::map<std::string, FileInfo>::iterator iter;

    // obtener el registro de este archivo desde _files
    if( ( iter = _files.find(std::string(fileName)) ) == _files.end() )
    {
        return -4;	// no puede ser eliminado ya que archivo no existe
    }

    // verificar que archivo a eliminar no está abierto
    if( iter->second._numOpen != 0 )
    {
        return -5;	// error: intento por eliminar archivo fallido
    }

    // eliminar archivo de sistema
    if( remove(fileName) != 0 )
        return -6;	//system is unable to delete a file

    // eliminar registro desde _files
    _files.erase(iter);

    // eliminación de archivo exitosa, retornar 0
    return 0;
}

RC PagedFileManager::openFile(const char *fileName, FileHandle &fileHandle)
{
    // verificar nombres de archivo ilegales
    if( fileName == NULL || strlen(fileName) == 0 )
    {
        return -14;
    }

    // si archivo no existe, entonces abortar
    if( !isExisting(fileName) )
    {
        return -4;	// error: archivo no existe
    }

    // iterador que debería referenciar al item en in _files
    std::map<std::string, FileInfo>::iterator iter;

    // obtener el registro de este archivo desde _files
    if( ( iter = _files.find(std::string(fileName)) ) == _files.end() )
    {
        return -7;	// error: registro no encontrado
    }

    // verificar que manejador de archivo no está siendo usado por otro archivo
    if( fileHandle._info != NULL || fileHandle._filePtr != NULL )
    {
        return -8;	// error: manejador de archivo es usado por otro archivo
    }

    // abrir un archivo binario para lectura y escritura
    FILE* file_ptr = fopen(fileName, "rb+");

    // verificar que el archivo fue creado exitosamente
    if( !file_ptr )
    {
        return -2;	// error: sistema no puede crear nuevo archivo
    }

    // asignar información para el manejador de archivo
    fileHandle._info = &(iter->second);
    fileHandle._filePtr = file_ptr;

    // incrementar contador de instancias abiertas del archivo
    (fileHandle._info->_numOpen)++;

    // archivo abierto exitosamente, retornar 0
    return 0;
}

RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    // verificar que fileHandle referencia a un archivo
    if( fileHandle._filePtr == NULL || fileHandle._info == NULL )
    {
        return -9;
    }

    // decrementar contador de numero de instancias abiertas del archivo
    (fileHandle._info->_numOpen)--;

    // cerrar archivo
    fclose(fileHandle._filePtr);

    // resetear atributos de fileHandler
    fileHandle._filePtr = NULL;
    fileHandle._info = NULL;

    // archivo cerrado exitosamente, retornar 0
    return 0;
}


FileHandle::FileHandle()
        : _info(NULL), _filePtr(NULL)
{
}


FileHandle::~FileHandle()
{
    _info = NULL;
    _filePtr = NULL;
}



RC FileHandle::readPage(PageNum pageNum, void *data)
{
    // verificar que el manejador de archivo referencia a algún archivo
    if( _filePtr == NULL || _info == NULL )
    {
        return -9;
    }

    // verificar que pageNum está dentro de los márgenes del archivo
    if( pageNum >= getNumberOfPages() )
    {
        return -10;
    }

    // verificar que data no está corrupta, es decir data no es null
    if( data == NULL )
    {
        return -11;
    }

    long int size = PAGE_SIZE * pageNum;

    // ir a la página especificada
    if( fseek(_filePtr, size, SEEK_SET) != 0 )
    {
        return -12; // error: fallo en fseek
    }

    // intentar leer PAGE_SIZE bytes
    size_t numBytes = fread(data, 1, PAGE_SIZE, _filePtr);

    // verificar que el número de bytes leídos es mayor a 0
    return (numBytes > 0) ? 0 : -13;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    // verificar que el manejador de archivo referencia a algún archivo
    if( _filePtr == NULL || _info == NULL )
    {
        return -9;
    }

    // verificar que pageNum está dentro de los márgenes del archivo
    if( pageNum >= getNumberOfPages() )
    {
        return -10;
    }

    // verificar que data no está corrupta, es decir data no es null
    if( data == NULL )
    {
        return -11;
    }

    // ir a la página especificada
    if( fseek(_filePtr, PAGE_SIZE * pageNum, SEEK_SET) != 0 )
    {
        return -12; // error: fallo en fseek
    }

    // intentar escribir PAGE_SIZE bytes
    size_t numBytes = fwrite(data, 1, PAGE_SIZE, _filePtr);

    // verificar que el número de bytes escritos es mayor a 0
    return (numBytes == PAGE_SIZE) ? 0 : -13;
}

RC FileHandle::appendPage(const void *data)
{
    // verificar que data no está corrupta, es decir data no es null
    if( data == NULL )
    {
        return -11;
    }

    // verificar que el manejador de archivo referencia a algún archivo
    if( _filePtr == NULL || _info == NULL )
    {
        return -9;
    }

    // ir a la última página del archivo
    if( fseek(_filePtr, 0, SEEK_END) != 0 )
    {
        return -12; // error: fallo en fseek
    }

    // escribir datos
    size_t numBytes = fwrite(data, 1, PAGE_SIZE, _filePtr);

    // verificar que el número de bytes escritos es igual al tamaño de página
    if( numBytes != PAGE_SIZE )
    {
        return -13; // error: fallo en fwrite
    }

    // incrementar contador de número de páginas
    _info->_numPages++;

    // éxito, retornar 0
    return 0;
}

unsigned FileHandle::getNumberOfPages()
{
    // verificar que el manejador de archivo referencia a algún archivo
    if( _info == NULL )
    {
        return -15;	// error: número de páginas desconocidos
    }

    // retornar número de páginas
    return _info->_numPages;
}


FileInfo::FileInfo(std::string name, unsigned int numOpen, unsigned int numPages)
        : _name(name), _numOpen(numOpen), _numPages(numPages)
{
}

FileInfo::~FileInfo()
{
}
