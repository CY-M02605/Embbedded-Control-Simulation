/**
 * @file manager.h
 * @brief Module manager using composite pattern
 * @date 27.05.2026
 */

#ifndef MANAGER_H
#define MANAGER_H

// Do not use "using namespace std" in header file.
// If declared in a header, it will force every file that includes this header
// to use the entire std namespace, which may lead to name conflicts.
// using namespace std;

#include "module_interface.h"

#include <vector>

namespace framework {
class Manager {
    public:
        // Reference version: does not accept the nullptr, therefore safer.
        void RegisterModule(ModuleInterface& modules) {
            module_.push_back(&modules);
        };  

        // Pointer version: nullptr may be passed, so null check is required/ necessary.
        // void RegisterModule(ModuleInterface* modules) {
        //     module_.push_back(modules);
        // };  

        void UpdateAll() {
            for (size_t i = 0; i < module_.size(); ++i) {
                module_[i]->Update();
            }
        }

    private:
        std::vector<ModuleInterface*> module_;
};
}

#endif
