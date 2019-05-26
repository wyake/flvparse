


LinkedList LinkedListInit()  
{  
    Node *L;  
    L = (Node *)malloc(sizeof(Node));   //申请结点空间   
    if(L == NULL)                       //判断是否有足够的内存空间   
        printf("申请内存空间失败/n");  
    L->next = NULL;                  //将next设置为NULL,初始长度为0的单链表   
}  
////////////////////////////////////////////   
//单链表的建立1，头插法建立单链表  
LinkedList LinkedListCreatH()  
{  
    Node *L;  
    L = (Node *)malloc(sizeof(Node));   //申请头结点空间  
    L->next = NULL;                      //初始化一个空链表  
      
    ElemType x;                         //x为链表数据域中的数据  
    while(scanf("%d",&x) != EOF)  
    {  
        Node *p;  
        p = (Node *)malloc(sizeof(Node));   //申请新的结点   
        p->data = x;                     //结点数据域赋值   
        p->next = L->next;                    //将结点插入到表头L-->|2|-->|1|-->NULL   
        L->next = p;   
    }  
    return L;   
}   
////////////////////////////////////////////   
//单链表的建立2，尾插法建立单链表  
LinkedList LinkedListCreatT()  
{  
    Node *L;  
    L = (Node *)malloc(sizeof(Node));   //申请头结点空间  
    L->next = NULL;                  //初始化一个空链表  
    Node *r;  
    r = L;                          //r始终指向终端结点，开始时指向头结点   
    ElemType x;                         //x为链表数据域中的数据  
    while(scanf("%d",&x) != EOF)  
    {  
        Node *p;  
        p = (Node *)malloc(sizeof(Node));   //申请新的结点   
        p->data = x;                     //结点数据域赋值   
        r->next = p;                 //将结点插入到表头L-->|1|-->|2|-->NULL   
        r = p;   
    }  
    r->next = NULL;   
      
    return L;     
}  
////////////////////////////////////////////   
//单链表的插入，在链表的第i个位置插入x的元素  
LinkedList LinkedListInsert(LinkedList L,int i,ElemType x)  
{  
    Node *pre;                      //pre为前驱结点   
    pre = L;  
    int tempi = 0;  
    for (tempi = 1; tempi < i; tempi++)  
        pre = pre->next;                 //查找第i个位置的前驱结点   
    Node *p;                                //插入的结点为p  
    p = (Node *)malloc(sizeof(Node));  
    p->data = x;   
    p->next = pre->next;  
    pre->next = p;  
      
    return L;                             
}   
////////////////////////////////////////////   
//单链表的删除，在链表中删除值为x的元素  
LinkedList LinkedListDelete(LinkedList L,ElemType x)  
{  
    Node *p,*pre;                   //pre为前驱结点，p为查找的结点。   
    p = L->next;  
    while(p->data != x)              //查找值为x的元素   
    {     
        pre = p;   
        p = p->next;  
    }  
    pre->next = p->next;          //删除操作，将其前驱next指向其后继。   
    free(p);  
    return L;  
}   