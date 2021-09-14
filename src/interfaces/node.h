#ifndef BTCU_NODE_H
#define BTCU_NODE_H

#include "../guiinterface.h"

class Node
{
public:
    Node() {}
    ~Node() {}

    void showProgress(const std::string& title, int progress)
    {
        uiInterface.ShowProgress(title, progress);
    }
};

#endif //BTCU_NODE_H
