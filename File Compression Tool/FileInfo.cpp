#include "FileInfo.h"

std::unordered_map<BYTE, std::size_t>* SelectedFileInfo::globalSymbolFrequency = new std::unordered_map<BYTE, std::size_t>;
std::size_t SelectedFileInfo::selectedFileAmount = 0;
std::size_t InternalFileInfo::folderAmount = 0;