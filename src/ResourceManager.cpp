#include "ResourceManager.h"

#include "Universe.h"

#include "Debug.h"

ResourceFile * ResourceManager::resourceFile = NULL;

std::map<std::string, World*> ResourceManager::s_worlds;
std::map<std::string, Map*> ResourceManager::s_maps;
std::map<std::string, Graphic*> ResourceManager::s_graphics;

Universe * ResourceManager::loadUniverse(std::string resourceFilePath, std::string id) {
    resourceFile = new ResourceFile(resourceFilePath);
    Universe * universe = new Universe(resourceFile->getResource(id));
    delete resourceFile;
    resourceFile = NULL;
    return universe;
}

#define RESOURCE_GETTER(TYPE_TITLECASE, TYPE_LOWERCASE, TYPE_QUOTED) \
TYPE_TITLECASE * ResourceManager::get##TYPE_TITLECASE(std::string id) { \
    Debug::assert(resourceFile != NULL, "ResourceManager::get" TYPE_QUOTED ": resourceFile == NULL"); \
    TYPE_TITLECASE * TYPE_LOWERCASE = find(s_##TYPE_LOWERCASE##s, id); \
    if (TYPE_LOWERCASE == NULL) { \
        const char * buffer = resourceFile->getResource(id); \
        if (buffer == NULL) { \
            std::cerr << "unable to load " TYPE_QUOTED ": " << id << std::endl; \
            return NULL; \
        } \
        TYPE_LOWERCASE = new TYPE_TITLECASE(buffer); \
        delete[] buffer; \
    } \
    return TYPE_LOWERCASE; \
}

RESOURCE_GETTER(World, world, "World")
RESOURCE_GETTER(Map, map, "Map")
RESOURCE_GETTER(Graphic, graphic, "Graphic")

