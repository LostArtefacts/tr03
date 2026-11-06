#ifndef TEMPLATE_H
#define TEMPLATE_H

struct STRING_FLAGGED {
    char *lpString;
    bool isPresented;
    STRING_FLAGGED(int size = 256)
    {
        lpString = new char[size];
        lpString[0] = 0;
        isPresented = true;
    }
    operator char *()
    {
        return lpString;
    }
    ~STRING_FLAGGED()
    {
        if (isPresented)
            delete[] lpString;
    }
};

template <class T> struct LIST {
    struct NODE {
        NODE *next;
        NODE *previous;
        T body;
        NODE(NODE *prevNode, NODE *nextNode) : next(nextNode), previous(prevNode)
        {
        }
    };
    NODE *head;
    NODE *tail;
    int count;
    LIST()
    {
        head = 0;
        tail = 0;
        count = 0;
    }
    ~LIST()
    {
        RemoveAll();
    }
    void RemoveAll()
    {
        NODE *node;
        NODE *next;

        for (node = head; node; node = next) {
            next = node->next;
            delete node;
        }
        head = 0;
        tail = 0;
        count = 0;
    }
    NODE *GetHead()
    {
        return head;
    }
    NODE *GetNext(NODE *node)
    {
        if (node)
            node = node->next;
        return node;
    }
    T &operator[](NODE *node)
    {
        return node->body;
    }
    T &NewTail()
    {
        NODE *node = new NODE(tail, 0);

        if (tail)
            tail->next = node;
        else
            head = node;
        tail = node;
        count++;
        return node->body;
    }
};

struct GENERIC_ADAPTER {
    GUID *lpGuid;
    GUID guid;
    STRING_FLAGGED description;
    STRING_FLAGGED name;
    void SetGUID(GUID *lpNewGuid)
    {
        if (lpNewGuid) {
            guid = *lpNewGuid;
            lpGuid = &guid;
        } else {
            memset(&guid, 0, sizeof(GUID));
            lpGuid = 0;
        }
    }
};

#endif
