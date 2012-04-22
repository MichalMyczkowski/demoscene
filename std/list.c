#include "std/debug.h"
#include "std/list.h"
#include "std/memory.h"

static NodeT *GetNode(ListT *list) {
  return list ? (&list->node) : NULL;
}

static PtrT GetData(NodeT *node) {
  return node ? ((ListT *)node)->data : NULL;
}

static NodeT *NodeAlloc(PtrT data) {
  ListT *item = MemNew0(sizeof(ListT));
  item->data = data;
  return GetNode(item);
}

static PtrT NodeFree(NodeT *node) {
  PtrT data = GetData(node);
  MemFree(node);
  return data;
}

ListT *NewList() {
  ListT *list = MemNew0(sizeof(ListT));
  NodeInitGuard(GetNode(list));
  return list;
}

void ResetList(ListT *list) {
  if (list) {
    NodeForEach(GetNode(list), (IterFuncT)MemFree, NULL);
    NodeInitGuard(GetNode(list));
  }
}

void DeleteList(ListT *list) {
  if (list) {
    NodeForEach(GetNode(list), (IterFuncT)MemFree, NULL);
    MemFree(list);
  }
}

void DeleteListFull(ListT *list, FreeFuncT func) {
  if (list) {
    ListForEach(list, (IterFuncT)func, NULL);
    DeleteList(list);
  }
}

void ListForEach(ListT *list, IterFuncT func, PtrT data) {
  NodeT *guard = GetNode(list);
  NodeT *node = guard->next;

  while (node != guard) {
    func(GetData(node), data);
    node = node->next;
  }
}

PtrT ListSearch(ListT *list, SearchFuncT func, PtrT data) {
  NodeT *guard = GetNode(list);
  NodeT *node = guard->next;

  while (node != guard) {
    if (!func(GetData(node), data))
      break;

    node = node->next;
  }

  return (node != guard) ? GetData(node) : NULL;
}

PtrT ListRemove(ListT *list, SearchFuncT func, PtrT data) {
  return NodeFree(NodeUnlink(ListSearch(list, func, data)));
}

PtrT ListGetNth(ListT *list, ssize_t index) {
  return GetData(NodeGetNth(GetNode(list), index));
}

PtrT ListPopNth(ListT *list, ssize_t index) {
  return NodeFree(NodeGetNth(GetNode(list), index));
}

void ListInsertAt(ListT *list, PtrT data, ssize_t index) {
  NodePrepend(NodeGetNth(GetNode(list), index), NodeAlloc(data));
}

PtrT ListPopBack(ListT *list) {
  return NodeFree(NodeUnlink(GetNode(list)));
}

PtrT ListPopFront(ListT *list) {
  return NodeFree(NodeUnlink(GetNode(list)));
}

void ListPushFront(ListT *list, PtrT data) {
  NodeAppend(GetNode(list), NodeAlloc(data));
}

void ListPushBack(ListT *list, PtrT data) {
  NodePrepend(GetNode(list), NodeAlloc(data));
}

size_t ListSize(ListT *list) {
  NodeT *guard = GetNode(list);
  NodeT *node = guard->next;
  size_t size = 0;

  while (node != guard) {
    node = node->next;
    size++;
  }

  return size;
}
