/**************************************************************************/
/*                                                                        */
/*            Copyright (c) 1996-2019 by Express Logic Inc.               */
/*                                                                        */
/*  This software is copyrighted by and is the sole property of Express   */
/*  Logic, Inc.  All rights, title, ownership, or other interests         */
/*  in the software remain the property of Express Logic, Inc.  This      */
/*  software may only be used in accordance with the corresponding        */
/*  license agreement.  Any unauthorized use, duplication, transmission,  */
/*  distribution, or disclosure of this software is expressly forbidden.  */
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */
/*  written consent of Express Logic, Inc.                                */
/*                                                                        */
/*  Express Logic, Inc. reserves the right to modify this software        */
/*  without notice.                                                       */
/*                                                                        */
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   Elliptic-curve Diffie-Hellman (ECDH)                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_ecdh.h                                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto ECDH module.                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_ECDH_H
#define NX_CRYPTO_ECDH_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"
#include "nx_crypto_ec.h"


/* Max Elliptic-curve Diffie-Hellman key size. Buffer size for calculations is 4X the key size */
#define NX_CRYPTO_ECDH_MAX_KEY_SIZE     (68)
#ifndef NX_CRYPTO_ECDH_SCRATCH_BUFFER_SIZE
#define NX_CRYPTO_ECDH_SCRATCH_BUFFER_SIZE 2464
#endif /* NX_CRYPTO_ECDSA_SCRATCH_BUFFER_SIZE */

/* Diffie-Hellman Key-exchange control structure. */
typedef struct NX_CRYPTO_ECDH_STRUCT
{
    /* The size of the key being used. This is primarily for testing, but also allows for future expansion.
       The value is assigned in _nx_crypto_dh_setup depending on the chosen group. */
    UINT nx_crypto_ecdh_key_size;

    /* The private key is generated by nx_crypto_dh_setup and is a random number.
       Make the array in units of UINT to make sure the starting address is 4-byte aligned. */
    HN_UBASE nx_crypto_ecdh_private_key_buffer[NX_CRYPTO_ECDH_MAX_KEY_SIZE >> HN_SIZE_SHIFT];

    /* The elliptic curve selected in the call to nx_crypto_ecdh_setup.  */
    NX_CRYPTO_EC *nx_crypto_ecdh_curve;

    HN_UBASE      nx_crypto_ecdh_scratch_buffer[NX_CRYPTO_ECDH_SCRATCH_BUFFER_SIZE >> HN_SIZE_SHIFT];
} NX_CRYPTO_ECDH;

/* Function prototypes */


UINT _nx_crypto_ecdh_key_pair_import(NX_CRYPTO_ECDH  *ecdh_ptr,
                                     NX_CRYPTO_EC *curve,
                                     UCHAR  *local_private_key_ptr,
                                     ULONG   local_private_key_len,
                                     UCHAR  *local_public_key_ptr,
                                     ULONG   local_public_key_len);

UINT _nx_crypto_ecdh_private_key_export(NX_CRYPTO_ECDH  *ecdh_ptr,
                                        UCHAR  *local_private_key_ptr,
                                        ULONG   local_private_key_len,
                                        ULONG  *actual_local_private_key_len);

UINT _nx_crypto_ecdh_setup(NX_CRYPTO_ECDH  *ecdh_ptr,
                           UCHAR  *local_public_key_ptr,
                           ULONG   local_public_key_len_ptr,
                           ULONG  *actual_local_public_key_len,
                           NX_CRYPTO_EC *curve,
                           HN_UBASE *scratch_buf_ptr);

UINT _nx_crypto_ecdh_compute_secret(NX_CRYPTO_ECDH  *ecdh_ptr,
                                    UCHAR  *share_secret_key_ptr,
                                    ULONG   share_secret_key_len_ptr,
                                    ULONG  *actual_share_secret_key_len,
                                    UCHAR  *remote_public_key,
                                    ULONG   remote_public_key_len,
                                    HN_UBASE *scratch_buf_ptr);

UINT _nx_crypto_method_ecdh_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                 UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                 VOID  **handle,
                                 VOID  *crypto_metadata,
                                 ULONG crypto_metadata_size);

UINT _nx_crypto_method_ecdh_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_ecdh_operation(UINT op,
                                      VOID *handle,
                                      struct NX_CRYPTO_METHOD_STRUCT *method,
                                      UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                      UCHAR *input, ULONG input_length_in_byte,
                                      UCHAR *iv_ptr,
                                      UCHAR *output, ULONG output_length_in_byte,
                                      VOID *crypto_metadata, ULONG crypto_metadata_size,
                                      VOID *packet_ptr,
                                      VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));

#ifdef __cplusplus
}
#endif

#endif /* NX_CRYPTO_ECDH_H */

