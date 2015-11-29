/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.20 $ of : mfd-data-get.m2c,v $
 *
 * $Id: usmDHUserKeyTable_data_get.h 1786 2008-11-10 13:36:41Z richie $
 *
 * @file usmDHUserKeyTable_data_get.h
 *
 * @addtogroup get
 *
 * Prototypes for get functions
 *
 * @{
 */
#ifndef USMDHUSERKEYTABLE_DATA_GET_H
#define USMDHUSERKEYTABLE_DATA_GET_H

#ifdef __cplusplus
extern          "C" {
#endif

    /*
     *********************************************************************
     * GET function declarations
     */

#ifdef NEED_USMDH_FUNCTIONS
    int             usmDHGetUserKeyChange(struct usmUser *user,
                                          int for_auth_key, char **keyobj,
                                          size_t *keyobj_len);
    DH             *usmDHGetUserDHptr(struct usmUser *user,
                                      int for_auth_key);
#endif

    /*
     *********************************************************************
     * GET Table declarations
     */

/**********************************************************************
 **********************************************************************
 ***
 *** Table usmDHUserKeyTable
 ***
 **********************************************************************
 **********************************************************************/
    /*
     * SNMP-USM-DH-OBJECTS-MIB::usmDHUserKeyTable is subid 2 of usmDHPublicObjects.
     * Its status is Current.
     * OID: .1.3.6.1.3.101.1.1.2, length: 9
     */
    /*
     * indexes
     */

    int             usmDHUserAuthKeyChange_get(usmDHUserKeyTable_rowreq_ctx
                                               * rowreq_ctx, char
                                               **usmDHUserAuthKeyChange_val_ptr_ptr, size_t
                                               *usmDHUserAuthKeyChange_val_ptr_len_ptr);
    int            
        usmDHUserOwnAuthKeyChange_get(usmDHUserKeyTable_rowreq_ctx *
                                      rowreq_ctx, char
                                      **usmDHUserOwnAuthKeyChange_val_ptr_ptr, size_t
                                      *usmDHUserOwnAuthKeyChange_val_ptr_len_ptr);
    int             usmDHUserPrivKeyChange_get(usmDHUserKeyTable_rowreq_ctx
                                               * rowreq_ctx, char
                                               **usmDHUserPrivKeyChange_val_ptr_ptr, size_t
                                               *usmDHUserPrivKeyChange_val_ptr_len_ptr);
    int            
        usmDHUserOwnPrivKeyChange_get(usmDHUserKeyTable_rowreq_ctx *
                                      rowreq_ctx, char
                                      **usmDHUserOwnPrivKeyChange_val_ptr_ptr, size_t
                                      *usmDHUserOwnPrivKeyChange_val_ptr_len_ptr);


    int            
        usmDHUserKeyTable_indexes_set_tbl_idx(usmDHUserKeyTable_mib_index *
                                              tbl_idx, char
                                              *usmUserEngineID_val_ptr, size_t
                                              usmUserEngineID_val_ptr_len,
                                              char *usmUserName_val_ptr,
                                              size_t
                                              usmUserName_val_ptr_len);
    int            
        usmDHUserKeyTable_indexes_set(usmDHUserKeyTable_rowreq_ctx *
                                      rowreq_ctx,
                                      char *usmUserEngineID_val_ptr,
                                      size_t usmUserEngineID_val_ptr_len,
                                      char *usmUserName_val_ptr,
                                      size_t usmUserName_val_ptr_len);




#ifdef __cplusplus
}
#endif
#endif                          /* USMDHUSERKEYTABLE_DATA_GET_H */
/** @} */
