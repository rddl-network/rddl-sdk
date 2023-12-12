#ifndef RDDL_ENUMS_H
#define RDDL_ENUMS_H


// the following values need to be inline with the tasmota/include/tasmota.h values of the variables
// from inside the SDK, the below mentioned variables are used
// tasmota.h enum is used from tasmota. these variable definitions lack the 'SDK_' prefix.
typedef enum {
    SDK_SET_PLANETMINT_API = 124,
    SDK_SET_PLANETMINT_CHAINID,
    SDK_SET_PLANETMINT_DENOM, 
    SDK_SET_NOTARIZTATION_PERIODICITY,
} RDDLSettings;

#endif /* RDDL_ENUMS_H */