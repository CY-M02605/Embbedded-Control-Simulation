/**
 * @file manager.h
 * @brief Manager for gear_display_facade in framework
 * @date 05.06.2026
 */

#ifndef MANAGER_H
#define MANAGER_H

#include <vector>

#include "module_interface.h"

namespace framework {
class Manager {
    public:
        // 2.Register a module into the manager
        // 2-1.Reference method: module is another name of each module
        
        // ↓ '&' in the parameter declaration means this parameter is a reference.
        void RegisterModule(ModuleInterface& module) {

            // ↓ '&' in the function body takes the address of the referenced object.
            module_.push_back(&module);
        }

        // 2-2.Pointer method: module is a variable that stores an address of each module name
        
        // * here is a definition of pointer variable
        // void RegisterModule(ModuleInterface* module) {

        //     // module without & ↓ actually is an address 
        //     module_.push_back(module);
        // }


        // 3.Call Update() for all registered modules
        void UpdateAll() {
            for (std::size_t i = 0; i < module_.size(); ++i) {
                module_[i]->Update();
            }
        }

    private:
        // 1.Store registered modules as base-class pointers

        // ↓ * here is a definition of pointer variable 
        std::vector<ModuleInterface*> module_;
};
}

#endif
