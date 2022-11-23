#ifndef GRAPHSORTER_H
#define GRAPHSORTER_H
#include "keira-global.h"

namespace keira {

class KEIRA_EXPORT GraphSorter
{
public:
    GraphSorter(NodeVector toSort);

    NodeVector sorted(){return m_sorted;}

private:
    void visit(Node *node);
    NodeVector m_toSort;
    NodeVector m_sorted;
};

} // namespace exo

#endif // GRAPHSORTER_H
