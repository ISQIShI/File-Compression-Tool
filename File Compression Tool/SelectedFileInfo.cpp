#include "SelectedFileInfo.h"

unordered_map<BYTE, size_t>* SelectedFileInfo::globalSymbolFrequency = new unordered_map<BYTE, size_t>;
size_t SelectedFileInfo::selectedFileAmount = 0;