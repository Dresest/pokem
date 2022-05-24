[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-c66648af7eb3fe8bc4f294546bfd86ef473780cde1dea487d3c4ff354943c9ae.svg)](https://classroom.github.com/online_ide?assignment_repo_id=7819221&assignment_repo_type=AssignmentRepo)
﻿

# Proyecto Base de Datos: Parte 1

**PagedFileManager:** provee funcionalidades para crear, eliminar, abrir y cerrar archivos con páginas. El sistema crea sólo una instancia de esta clase y todas las solicitudes se realizan a través de esta instancia. A continuación se muestra la definición de la clase y sus método. Cada método a excepción del constructor y destructor retorna un valor de retorno RC (return code: 0 indica ejecución normal).

    class PagedFileManager {

    public:
        // Acceso a la instancia _pf_manager
        static PagedFileManager* instance();                                      

        // Crea un nuevo archivo
        RC createFile    (const char *fileName);                            
        
        // Elimina un archivo
        RC destroyFile   (const char *fileName);                            

        // Abre un archivo
        RC openFile      (const char *fileName, FileHandle &fileHandle);

        // Cierra un archivo
        RC closeFile     (FileHandle &fileHandle);

    protected:    
        // Constructor
        PagedFileManager();

        // Destructor
        ~PagedFileManager();

    private:
        static PagedFileManager* _pf_manager;
    }



**FileHandle:** provee acceso a las p\'aginas de un archivo abierto. Para accesar a las p\'aginas de un archivo, un cliente debe primero create una instancia de esta clase y pasarla a PagedFileManager.

    class FileHandle {
    public:
        // Constructor
        FileHandle();

        // Destructor
        ~FileHandle();

        // Obtiene una página especñifica
        RC readPage(PageNum pageNum, void *data);

        // Escribe un página específica
        RC writePage(PageNum pageNum, const void *data);

        // Añade una página específica
        RC appendPage(const void *data);

        // Obtiene el número de páginas de un archivo
        unsigned getNumberOfPages();
    }

**RecordBasedFileManager:** maneja operaciones basadas en registros/tuplas tales como inserci\'on, eliminaci\'on, actualizaci\'on y lectura de registros. 

    class RecordBasedFileManager {
    public:
        // Acceso a la instancia _rbf_manager
        static RecordBasedFileManager* instance();                                      

        // Crea un nuevo archivo con estructura de registros
        RC createFile(const string &fileName);
    
        // Elimina un archivo con estructuras de registros
        RC destroyFile(const string &fileName);
    
        // Abre un archivo con estructuras de registros
        RC openFile(const string &fileName, FileHandle &fileHandle);
    
        // Cierra un archivo con estructuras de registros
        RC closeFile(FileHandle &fileHandle);
    
        // Inserta un registro en un archivo
        RC insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid);

        // Lee un registro dado un rid específico
        RC readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data);
    
        // Imprime el registro que es pasado por parámetro
        RC printRecord(const vector<Attribute> &recordDescriptor, const void *data);

        // Elimina todos los registros de un archivo
        RC deleteRecords(FileHandle &fileHandle);

        // Elimina el registro especificado en rid desde el archivo
        RC deleteRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid);
        
    protected:    
        // Constructor
        RecordBasedFileManager();

        // Destructor
        ~RecordBasedFileManager();

    private:
        static RecordBasedFileManager* _rbf_manager;
    }