// function prototypes
void execute(void (*somefunction)(void *p), void *p);
int pool_submit(void (*somefunction)(void *p), void *p);
void *worker();
void pool_init(void);
void pool_shutdown(void);

typedef struct Data
{
    int a;
    int b;
}data;
