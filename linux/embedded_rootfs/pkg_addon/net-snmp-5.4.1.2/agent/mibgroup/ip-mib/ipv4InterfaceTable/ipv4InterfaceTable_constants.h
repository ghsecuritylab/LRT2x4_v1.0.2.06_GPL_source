/*
 * Note: this file originally auto-generated by mib2c using
 *  : generic-table-constants.m2c,v 1.5 2005/07/15 22:41:16 rstory Exp $
 *
 * $Id: ipv4InterfaceTable_constants.h 1786 2008-11-10 13:36:41Z richie $
 */
#ifndef IPV4INTERFACETABLE_CONSTANTS_H
#define IPV4INTERFACETABLE_CONSTANTS_H

#ifdef __cplusplus
extern          "C" {
#endif


    /*
     * column number definitions for table ipv4InterfaceTable 
     */
#define IPV4INTERFACETABLE_OID              1,3,6,1,2,1,4,28
#define IPV4INTERFACETABLELASTCHANGE_OID    1,3,6,1,2,1,4,27

#define COLUMN_IPV4INTERFACEIFINDEX         1

#define COLUMN_IPV4INTERFACEREASMMAXSIZE         2

#define COLUMN_IPV4INTERFACEENABLESTATUS         3
#define COLUMN_IPV4INTERFACEENABLESTATUS_FLAG    (0x1 << 2)

#define COLUMN_IPV4INTERFACERETRANSMITTIME         4


#define IPV4INTERFACETABLE_MIN_COL   COLUMN_IPV4INTERFACEREASMMAXSIZE
#define IPV4INTERFACETABLE_MAX_COL   COLUMN_IPV4INTERFACERETRANSMITTIME


    /*
     * TODO:405:r: Review IPV4INTERFACETABLE_SETTABLE_COLS macro.
     * OR together all the writable cols.
     */
#define IPV4INTERFACETABLE_SETTABLE_COLS (COLUMN_IPV4INTERFACEENABLESTATUS_FLAG)
    /*
     * NOTES on enums
     * ==============
     *
     * Value Mapping
     * -------------
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them
     * below. For example, a boolean flag (1/0) is usually represented
     * as a TruthValue in a MIB, which maps to the values (1/2).
     *
     */
/*************************************************************************
 *************************************************************************
 *
 * enum definitions for table ipv4InterfaceTable
 *
 *************************************************************************
 *************************************************************************/

/*************************************************************
 * constants for enums for the MIB node
 * ipv4InterfaceEnableStatus (INTEGER / ASN_INTEGER)
 *
 * since a Textual Convention may be referenced more than once in a
 * MIB, protect againt redefinitions of the enum values.
 */
#ifndef IPV4INTERFACEENABLESTATUS_ENUMS
#define IPV4INTERFACEENABLESTATUS_ENUMS

#define IPV4INTERFACEENABLESTATUS_UP  1
#define IPV4INTERFACEENABLESTATUS_DOWN  2

#endif                          /* IPV4INTERFACEENABLESTATUS_ENUMS */




#ifdef __cplusplus
}
#endif
#endif                          /* IPV4INTERFACETABLE_OIDS_H */
