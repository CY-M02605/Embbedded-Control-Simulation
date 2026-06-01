/**
 * @file module_interface.h
 * @brief ModuleInterface for framework of engine management system
 * @date 29.05.2026
 */

 #ifndef MODULE_INTERFACE_H
 #define MODULE_INTERFACE_H

 namespace framework {
 class ModuleInterface {
    public:
        virtual ~ModuleInterface() {}

        virtual void Update() = 0;

        // void Init();

    private:
 };
 }


 #endif
