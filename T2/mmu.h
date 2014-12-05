

/* tipo para endereco virtual do MMU */
typedef int mmu_addr_t;


/* inicializa mmu com parametros recebidos*/
void initMMU(int wsl, int nFrames);

/* aloca uma pagina na memoria para a thread chamadora */
mmu_addr_t requestPage(int value);

/* imprime a memoria */
void printMemory(void);

/* imprime tabela virtual da thread chamadora */
void printVirtualPageTable(void);

/* obtem conteudo de uma pagina */
int getPageContent(mmu_addr_t virtualAddr);

/* libera espacos alocados por aquela thread */
void freeMySpaces(void);

/* destroi MMU */
void destroyMMU(void);