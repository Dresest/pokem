#ifndef _pfm_h_
#define _pfm_h_

#include <string>
#include <map>

typedef int RC;
typedef unsigned PageNum;

#define PAGE_SIZE 4096

class FileHandle;

/***
 * FileInfo mantiene información sobre un archivo
**/
class FileInfo
{
public:
    // NOTA: todos los métodos son públicos
    FileInfo(std::string name, unsigned int numOpen, unsigned int numPages);
    ~FileInfo();

public:
    // nombre de archivo incluyendo su ruta si la trea
    std::string _name;
    // cuántas veces está el archivo abierto. Si _numOpen == 0, entonces no está abierto
    unsigned int _numOpen;
    // número de páginas
    unsigned int _numPages;
};

class PagedFileManager
{
public:
    static PagedFileManager* instance(); // acceso a la instancia de _pf_manager

    bool isExisting(const char *fileName)	const;

    RC createFile    (const char *fileName);                         // crear un nuevo archivo
    RC destroyFile   (const char *fileName);                         // eliminar archivo
    RC openFile      (const char *fileName, FileHandle &fileHandle); // abrir un archivo
    RC closeFile     (FileHandle &fileHandle);                       // cerrar archivo
    RC findHeaderPage(FileHandle& fileHandle, PageNum pageId, PageNum& retHeaderPage);

protected:
    PagedFileManager();      // constructor
    ~PagedFileManager();     // destructor

private:
    static PagedFileManager *_pf_manager;
    // hash map que almacena información sobre todos los archivos
    std::map<std::string, FileInfo> _files;
};


class FileHandle
{
public:
    FileHandle();             // constructor
    ~FileHandle();            // destructor

    RC readPage(PageNum pageNum, void *data);            // obtener una página específica
    RC writePage(PageNum pageNum, const void *data);     // escribir una página específica
    RC appendPage(const void *data);                     // añadir una página específica
    unsigned getNumberOfPages();                         // obtener el número de páginas del archivo

public:
    // referencia a la información del archivo
    FileInfo* _info;
    // referencia al manejador de archivos del sistema operativo
    FILE* _filePtr;
};



#endif
