/**
 * @file manager.h
 * @brife Manager class for framework of engine management system
 * @date 31.5.2026
 */

 #ifndef MANAGER_H
 #define MANAGER_H

 #include "module_interface.h"
 #include <vector>

 namespace framework {
 class Manager {
   public:
        void RegisterModule(ModuleInterface& module) {
            module_.push_back(&module);
        }

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
