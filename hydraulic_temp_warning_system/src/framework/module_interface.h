/**
 * @file module_interface.h
 * @brief Module interface base class for the framework
 * @date 27.05.2026
 */

#ifndef MODULE_INTERFACE_H
#define MODULE_INTERFACE_H

namespace framework {
class ModuleInterface {
public:
    virtual ~ModuleInterface() {}

    virtual void Update() = 0;
};
}

#endif
