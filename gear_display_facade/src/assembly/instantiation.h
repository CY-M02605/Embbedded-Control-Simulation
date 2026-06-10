#ifndef INSTANTIATION_H
#define INSTANTIATION_H
#endif 

#undef INSTANTIATION

#if defined(INSTANTIATION_DEFINE)
#define INSTANTIATION(Type, name, ...) Type name{__VA_ARGS__};
#else
#define INSTANTIATION(Type, name, ...) extern Type name;
#endif
