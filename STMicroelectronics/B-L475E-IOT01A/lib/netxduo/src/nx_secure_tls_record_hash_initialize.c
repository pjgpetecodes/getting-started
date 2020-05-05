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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_record_hash_initialize               PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes a hash method for generating a MAC for    */
/*    a TLS message during the encrypted portion of the TLS handshake and */
/*    session. The hashing must be done in pieces to support the packet   */
/*    chaining feature of NX_PACKET.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    sequence_num                          Record sequence number        */
/*    header                                Record header                 */
/*    header_length                         Length of record header       */
/*    hash_length                           Length of hash                */
/*    mac_secret                            Key used for MAC generation   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_send_record            Send the TLS record           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            logic to properly initialize*/
/*                                            the crypto control blcok,   */
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_record_hash_initialize(NX_SECURE_TLS_SESSION *tls_session,
                                           ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                           UCHAR *header, UINT header_length, UINT *hash_length,
                                           UCHAR *mac_secret)
{
UINT                                  status;
NX_CRYPTO_METHOD                     *authentication_method;
UINT                                  hash_size;
UCHAR                                 adjusted_sequence_num[8];



    /* We need to generate a Message Authentication Code (MAC) for each record during an "active" TLS session
       (following a ChangeCipherSpec message). The hash algorithm is determined by the ciphersuite, and HMAC
       is used with that hash algorithm to protect the TLS record contents from tampering.

       The MAC is generated as:

       HMAC_hash(MAC_write_secret, seq_num + type + version + length + data);

     */

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get our authentication method from the ciphersuite. */
    authentication_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;

    /* Assert that we have an authentication mechanism. */
    NX_ASSERT(authentication_method -> nx_crypto_operation != NX_NULL);

    /* Get the hash size and MAC secret for our current session. */
    hash_size = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size;

    /* Correct the endianness of our sequence number before hashing. */
    adjusted_sequence_num[0] = (UCHAR)(sequence_num[1] >> 24);
    adjusted_sequence_num[1] = (UCHAR)(sequence_num[1] >> 16);
    adjusted_sequence_num[2] = (UCHAR)(sequence_num[1] >> 8);
    adjusted_sequence_num[3] = (UCHAR)(sequence_num[1]);
    adjusted_sequence_num[4] = (UCHAR)(sequence_num[0] >> 24);
    adjusted_sequence_num[5] = (UCHAR)(sequence_num[0] >> 16);
    adjusted_sequence_num[6] = (UCHAR)(sequence_num[0] >> 8);
    adjusted_sequence_num[7] = (UCHAR)(sequence_num[0]);

    if (authentication_method -> nx_crypto_init)
    {
        status = authentication_method -> nx_crypto_init(authentication_method,
                                                         mac_secret,
                                                         (NX_CRYPTO_KEY_SIZE)(hash_size << 3),
                                                         &tls_session -> nx_secure_hash_mac_handler,
                                                         tls_session -> nx_secure_hash_mac_metadata_area,
                                                         tls_session -> nx_secure_hash_mac_metadata_size);

        if (status != NX_SUCCESS)
        {
            return(status);
        }
    }

    /* Call the initialization routine for our hash method. */
    status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                          tls_session -> nx_secure_hash_mac_handler,
                                                          authentication_method,
                                                          mac_secret,
                                                          (NX_CRYPTO_KEY_SIZE)(hash_size << 3),
                                                          NX_NULL,
                                                          0,
                                                          NX_NULL,
                                                          NX_NULL,
                                                          0,
                                                          tls_session -> nx_secure_hash_mac_metadata_area,
                                                          tls_session -> nx_secure_hash_mac_metadata_size,
                                                          NX_NULL,
                                                          NX_NULL);

    /* Check for error in encryption routine. */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Update the hash with the sequence number. */
    status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                          tls_session -> nx_secure_hash_mac_handler,
                                                          authentication_method,
                                                          NX_NULL,
                                                          0,
                                                          adjusted_sequence_num,
                                                          8,
                                                          NX_NULL,
                                                          NX_NULL,
                                                          0,
                                                          tls_session -> nx_secure_hash_mac_metadata_area,
                                                          tls_session -> nx_secure_hash_mac_metadata_size,
                                                          NX_NULL,
                                                          NX_NULL);

    /* Check for error in encryption routine. */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(adjusted_sequence_num, 0, sizeof(adjusted_sequence_num));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Update the hash with the record header. */
    status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                          tls_session -> nx_secure_hash_mac_handler,
                                                          authentication_method,
                                                          NX_NULL,
                                                          0,
                                                          header,
                                                          header_length,
                                                          NX_NULL,
                                                          NX_NULL,
                                                          0,
                                                          tls_session -> nx_secure_hash_mac_metadata_area,
                                                          tls_session -> nx_secure_hash_mac_metadata_size,
                                                          NX_NULL,
                                                          NX_NULL);


    /* Return how many bytes our hash is since the caller doesn't necessarily know. */
    *hash_length = hash_size;

    return(status);
}

