/***************************************************************************
                             pabtypes.h  -  description
                             -------------------
    begin                : Wed Aug 2 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 * The information in this header file was found in a VC Header file.      *
 * It's probably a header file that can be used to provide parameters or   *
 * stearing information to MAPI functions.                                 *
 *                                                                         *
 * It turns out that the values below are the type information we searched *
 * for in the .PAB file. So we can use them to read the format. Of course  *
 * we could have figured this out by ourselves, and we had figured out a   *
 * great deal, but why do a lot of work, if you can read it somewhere else?*
 *                                                                         *
 ***************************************************************************/


#ifndef _MAPITAGS_H_
#define _MAPITAGS_H_

/*
 *  Message envelope
 */

#define PR_ACKNOWLEDGEMENT_MODE                     0x0001
#define PR_ALTERNATE_RECIPIENT_ALLOWED              0x0002
#define PR_AUTHORIZING_USERS                        0x0003
#define PR_AUTO_FORWARD_COMMENT                     0x0004
#define PR_AUTO_FORWARD_COMMENT_W                   0x0004
#define PR_AUTO_FORWARD_COMMENT_A                   0x0004
#define PR_AUTO_FORWARDED                           0x0005
#define PR_CONTENT_CONFIDENTIALITY_ALGORITHM_ID     0x0006
#define PR_CONTENT_CORRELATOR                       0x0007
#define PR_CONTENT_IDENTIFIER                       0x0008
#define PR_CONTENT_IDENTIFIER_W                     0x0008
#define PR_CONTENT_IDENTIFIER_A                     0x0008
#define PR_CONTENT_LENGTH                           0x0009
#define PR_CONTENT_RETURN_REQUESTED                 0x000A



#define PR_CONVERSATION_KEY                         0x000B

#define PR_CONVERSION_EITS                          0x000C
#define PR_CONVERSION_WITH_LOSS_PROHIBITED          0x000D
#define PR_CONVERTED_EITS                           0x000E
#define PR_DEFERRED_DELIVERY_TIME                   0x000F
#define PR_DELIVER_TIME                             0x0010
#define PR_DISCARD_REASON                           0x0011
#define PR_DISCLOSURE_OF_RECIPIENTS                 0x0012
#define PR_DL_EXPANSION_HISTORY                     0x0013
#define PR_DL_EXPANSION_PROHIBITED                  0x0014
#define PR_EXPIRY_TIME                              0x0015
#define PR_IMPLICIT_CONVERSION_PROHIBITED           0x0016
#define PR_IMPORTANCE                               0x0017
#define PR_IPM_ID                                   0x0018
#define PR_LATEST_DELIVERY_TIME                     0x0019
#define PR_MESSAGE_CLASS                            0x001A
#define PR_MESSAGE_CLASS_W                          0x001A
#define PR_MESSAGE_CLASS_A                          0x001A
#define PR_MESSAGE_DELIVERY_ID                      0x001B





#define PR_MESSAGE_SECURITY_LABEL                   0x001E
#define PR_OBSOLETED_IPMS                           0x001F
#define PR_ORIGINALLY_INTENDED_RECIPIENT_NAME       0x0020
#define PR_ORIGINAL_EITS                            0x0021
#define PR_ORIGINATOR_CERTIFICATE                   0x0022
#define PR_ORIGINATOR_DELIVERY_REPORT_REQUESTED     0x0023
#define PR_ORIGINATOR_RETURN_ADDRESS                0x0024



#define PR_PARENT_KEY                               0x0025
#define PR_PRIORITY                                 0x0026



#define PR_ORIGIN_CHECK                             0x0027
#define PR_PROOF_OF_SUBMISSION_REQUESTED            0x0028
#define PR_READ_RECEIPT_REQUESTED                   0x0029
#define PR_RECEIPT_TIME                             0x002A
#define PR_RECIPIENT_REASSIGNMENT_PROHIBITED        0x002B
#define PR_REDIRECTION_HISTORY                      0x002C
#define PR_RELATED_IPMS                             0x002D
#define PR_ORIGINAL_SENSITIVITY                     0x002E
#define PR_LANGUAGES                                0x002F
#define PR_LANGUAGES_W                              0x002F
#define PR_LANGUAGES_A                              0x002F
#define PR_REPLY_TIME                               0x0030
#define PR_REPORT_TAG                               0x0031
#define PR_REPORT_TIME                              0x0032
#define PR_RETURNED_IPM                             0x0033
#define PR_SECURITY                                 0x0034
#define PR_INCOMPLETE_COPY                          0x0035
#define PR_SENSITIVITY                              0x0036
#define PR_SUBJECT                                  0x0037
#define PR_SUBJECT_W                                0x0037
#define PR_SUBJECT_A                                0x0037
#define PR_SUBJECT_IPM                              0x0038
#define PR_CLIENT_SUBMIT_TIME                       0x0039
#define PR_REPORT_NAME                              0x003A
#define PR_REPORT_NAME_W                            0x003A
#define PR_REPORT_NAME_A                            0x003A
#define PR_SENT_REPRESENTING_SEARCH_KEY             0x003B
#define PR_X400_CONTENT_TYPE                        0x003C
#define PR_SUBJECT_PREFIX                           0x003D
#define PR_SUBJECT_PREFIX_W                         0x003D
#define PR_SUBJECT_PREFIX_A                         0x003D
#define PR_NON_RECEIPT_REASON                       0x003E
#define PR_RECEIVED_BY_ENTRYID                      0x003F
#define PR_RECEIVED_BY_NAME                         0x0040
#define PR_RECEIVED_BY_NAME_W                       0x0040
#define PR_RECEIVED_BY_NAME_A                       0x0040
#define PR_SENT_REPRESENTING_ENTRYID                0x0041
#define PR_SENT_REPRESENTING_NAME                   0x0042
#define PR_SENT_REPRESENTING_NAME_W                 0x0042
#define PR_SENT_REPRESENTING_NAME_A                 0x0042
#define PR_RCVD_REPRESENTING_ENTRYID                0x0043
#define PR_RCVD_REPRESENTING_NAME                   0x0044
#define PR_RCVD_REPRESENTING_NAME_W                 0x0044
#define PR_RCVD_REPRESENTING_NAME_A                 0x0044
#define PR_REPORT_ENTRYID                           0x0045
#define PR_READ_RECEIPT_ENTRYID                     0x0046
#define PR_MESSAGE_SUBMISSION_ID                    0x0047
#define PR_PROVIDER_SUBMIT_TIME                     0x0048
#define PR_ORIGINAL_SUBJECT                         0x0049
#define PR_ORIGINAL_SUBJECT_W                       0x0049
#define PR_ORIGINAL_SUBJECT_A                       0x0049
#define PR_DISC_VAL                                 0x004A
#define PR_ORIG_MESSAGE_CLASS                       0x004B
#define PR_ORIG_MESSAGE_CLASS_W                     0x004B
#define PR_ORIG_MESSAGE_CLASS_A                     0x004B
#define PR_ORIGINAL_AUTHOR_ENTRYID                  0x004C
#define PR_ORIGINAL_AUTHOR_NAME                     0x004D
#define PR_ORIGINAL_AUTHOR_NAME_W                   0x004D
#define PR_ORIGINAL_AUTHOR_NAME_A                   0x004D
#define PR_ORIGINAL_SUBMIT_TIME                     0x004E
#define PR_REPLY_RECIPIENT_ENTRIES                  0x004F
#define PR_REPLY_RECIPIENT_NAMES                    0x0050
#define PR_REPLY_RECIPIENT_NAMES_W                  0x0050
#define PR_REPLY_RECIPIENT_NAMES_A                  0x0050

#define PR_RECEIVED_BY_SEARCH_KEY                   0x0051
#define PR_RCVD_REPRESENTING_SEARCH_KEY             0x0052
#define PR_READ_RECEIPT_SEARCH_KEY                  0x0053
#define PR_REPORT_SEARCH_KEY                        0x0054
#define PR_ORIGINAL_DELIVERY_TIME                   0x0055
#define PR_ORIGINAL_AUTHOR_SEARCH_KEY               0x0056

#define PR_MESSAGE_TO_ME                            0x0057
#define PR_MESSAGE_CC_ME                            0x0058
#define PR_MESSAGE_RECIP_ME                         0x0059

#define PR_ORIGINAL_SENDER_NAME                     0x005A
#define PR_ORIGINAL_SENDER_NAME_W                   0x005A
#define PR_ORIGINAL_SENDER_NAME_A                   0x005A
#define PR_ORIGINAL_SENDER_ENTRYID                  0x005B
#define PR_ORIGINAL_SENDER_SEARCH_KEY               0x005C
#define PR_ORIGINAL_SENT_REPRESENTING_NAME          0x005D
#define PR_ORIGINAL_SENT_REPRESENTING_NAME_W        0x005D
#define PR_ORIGINAL_SENT_REPRESENTING_NAME_A        0x005D
#define PR_ORIGINAL_SENT_REPRESENTING_ENTRYID       0x005E
#define PR_ORIGINAL_SENT_REPRESENTING_SEARCH_KEY    0x005F

#define PR_START_DATE                               0x0060
#define PR_END_DATE                                 0x0061
#define PR_OWNER_APPT_ID                            0x0062
#define PR_RESPONSE_REQUESTED                       0x0063

#define PR_SENT_REPRESENTING_ADDRTYPE               0x0064
#define PR_SENT_REPRESENTING_ADDRTYPE_W             0x0064
#define PR_SENT_REPRESENTING_ADDRTYPE_A             0x0064
#define PR_SENT_REPRESENTING_EMAIL_ADDRESS          0x0065
#define PR_SENT_REPRESENTING_EMAIL_ADDRESS_W        0x0065
#define PR_SENT_REPRESENTING_EMAIL_ADDRESS_A        0x0065

#define PR_ORIGINAL_SENDER_ADDRTYPE                 0x0066
#define PR_ORIGINAL_SENDER_ADDRTYPE_W               0x0066
#define PR_ORIGINAL_SENDER_ADDRTYPE_A               0x0066
#define PR_ORIGINAL_SENDER_EMAIL_ADDRESS            0x0067
#define PR_ORIGINAL_SENDER_EMAIL_ADDRESS_W          0x0067
#define PR_ORIGINAL_SENDER_EMAIL_ADDRESS_A          0x0067

#define PR_ORIGINAL_SENT_REPRESENTING_ADDRTYPE      0x0068
#define PR_ORIGINAL_SENT_REPRESENTING_ADDRTYPE_W    0x0068
#define PR_ORIGINAL_SENT_REPRESENTING_ADDRTYPE_A    0x0068
#define PR_ORIGINAL_SENT_REPRESENTING_EMAIL_ADDRESS 0x0069
#define PR_ORIGINAL_SENT_REPRESENTING_EMAIL_ADDRESS_W   0x0069
#define PR_ORIGINAL_SENT_REPRESENTING_EMAIL_ADDRESS_A   0x0069

#define PR_CONVERSATION_TOPIC                       0x0070
#define PR_CONVERSATION_TOPIC_W                     0x0070
#define PR_CONVERSATION_TOPIC_A                     0x0070
#define PR_CONVERSATION_INDEX                       0x0071

#define PR_ORIGINAL_DISPLAY_BCC                     0x0072
#define PR_ORIGINAL_DISPLAY_BCC_W                   0x0072
#define PR_ORIGINAL_DISPLAY_BCC_A                   0x0072
#define PR_ORIGINAL_DISPLAY_CC                      0x0073
#define PR_ORIGINAL_DISPLAY_CC_W                    0x0073
#define PR_ORIGINAL_DISPLAY_CC_A                    0x0073
#define PR_ORIGINAL_DISPLAY_TO                      0x0074
#define PR_ORIGINAL_DISPLAY_TO_W                    0x0074
#define PR_ORIGINAL_DISPLAY_TO_A                    0x0074

#define PR_RECEIVED_BY_ADDRTYPE                     0x0075
#define PR_RECEIVED_BY_ADDRTYPE_W                   0x0075
#define PR_RECEIVED_BY_ADDRTYPE_A                   0x0075
#define PR_RECEIVED_BY_EMAIL_ADDRESS                0x0076
#define PR_RECEIVED_BY_EMAIL_ADDRESS_W              0x0076
#define PR_RECEIVED_BY_EMAIL_ADDRESS_A              0x0076

#define PR_RCVD_REPRESENTING_ADDRTYPE               0x0077
#define PR_RCVD_REPRESENTING_ADDRTYPE_W             0x0077
#define PR_RCVD_REPRESENTING_ADDRTYPE_A             0x0077
#define PR_RCVD_REPRESENTING_EMAIL_ADDRESS          0x0078
#define PR_RCVD_REPRESENTING_EMAIL_ADDRESS_W        0x0078
#define PR_RCVD_REPRESENTING_EMAIL_ADDRESS_A        0x0078

#define PR_ORIGINAL_AUTHOR_ADDRTYPE                 0x0079
#define PR_ORIGINAL_AUTHOR_ADDRTYPE_W               0x0079
#define PR_ORIGINAL_AUTHOR_ADDRTYPE_A               0x0079
#define PR_ORIGINAL_AUTHOR_EMAIL_ADDRESS            0x007A
#define PR_ORIGINAL_AUTHOR_EMAIL_ADDRESS_W          0x007A
#define PR_ORIGINAL_AUTHOR_EMAIL_ADDRESS_A          0x007A

#define PR_ORIGINALLY_INTENDED_RECIP_ADDRTYPE       0x007B
#define PR_ORIGINALLY_INTENDED_RECIP_ADDRTYPE_W     0x007B
#define PR_ORIGINALLY_INTENDED_RECIP_ADDRTYPE_A     0x007B
#define PR_ORIGINALLY_INTENDED_RECIP_EMAIL_ADDRESS  0x007C
#define PR_ORIGINALLY_INTENDED_RECIP_EMAIL_ADDRESS_W    0x007C
#define PR_ORIGINALLY_INTENDED_RECIP_EMAIL_ADDRESS_A    0x007C

#define PR_TRANSPORT_MESSAGE_HEADERS                0x007D
#define PR_TRANSPORT_MESSAGE_HEADERS_W              0x007D
#define PR_TRANSPORT_MESSAGE_HEADERS_A              0x007D

#define PR_DELEGATION                               0x007E

#define PR_TNEF_CORRELATION_KEY                     0x007F



/*
 *  Message content properties
 */

#define PR_BODY                                     0x1000
#define PR_BODY_W                                   0x1000
#define PR_BODY_A                                   0x1000
#define PR_REPORT_TEXT                              0x1001
#define PR_REPORT_TEXT_W                            0x1001
#define PR_REPORT_TEXT_A                            0x1001
#define PR_ORIGINATOR_AND_DL_EXPANSION_HISTORY      0x1002
#define PR_REPORTING_DL_NAME                        0x1003
#define PR_REPORTING_MTA_CERTIFICATE                0x1004

/*  Removed PR_REPORT_ORIGIN_AUTHENTICATION_CHECK with DCR 3865, use PR_ORIGIN_CHECK */

#define PR_RTF_SYNC_BODY_CRC                        0x1006
#define PR_RTF_SYNC_BODY_COUNT                      0x1007
#define PR_RTF_SYNC_BODY_TAG                        0x1008
#define PR_RTF_SYNC_BODY_TAG_W                      0x1008
#define PR_RTF_SYNC_BODY_TAG_A                      0x1008
#define PR_RTF_COMPRESSED                           0x1009
#define PR_RTF_SYNC_PREFIX_COUNT                    0x1010
#define PR_RTF_SYNC_TRAILING_COUNT                  0x1011
#define PR_ORIGINALLY_INTENDED_RECIP_ENTRYID        0x1012

/*
 *  Reserved 0x1100-0x1200
 */


/*
 *  Message recipient properties
 */

#define PR_CONTENT_INTEGRITY_CHECK                  0x0C00
#define PR_EXPLICIT_CONVERSION                      0x0C01
#define PR_IPM_RETURN_REQUESTED                     0x0C02
#define PR_MESSAGE_TOKEN                            0x0C03
#define PR_NDR_REASON_CODE                          0x0C04
#define PR_NDR_DIAG_CODE                            0x0C05
#define PR_NON_RECEIPT_NOTIFICATION_REQUESTED       0x0C06
#define PR_DELIVERY_POINT                           0x0C07

#define PR_ORIGINATOR_NON_DELIVERY_REPORT_REQUESTED 0x0C08
#define PR_ORIGINATOR_REQUESTED_ALTERNATE_RECIPIENT 0x0C09
#define PR_PHYSICAL_DELIVERY_BUREAU_FAX_DELIVERY    0x0C0A
#define PR_PHYSICAL_DELIVERY_MODE                   0x0C0B
#define PR_PHYSICAL_DELIVERY_REPORT_REQUEST         0x0C0C
#define PR_PHYSICAL_FORWARDING_ADDRESS              0x0C0D
#define PR_PHYSICAL_FORWARDING_ADDRESS_REQUESTED    0x0C0E
#define PR_PHYSICAL_FORWARDING_PROHIBITED           0x0C0F
#define PR_PHYSICAL_RENDITION_ATTRIBUTES            0x0C10
#define PR_PROOF_OF_DELIVERY                        0x0C11
#define PR_PROOF_OF_DELIVERY_REQUESTED              0x0C12
#define PR_RECIPIENT_CERTIFICATE                    0x0C13
#define PR_RECIPIENT_NUMBER_FOR_ADVICE              0x0C14
#define PR_RECIPIENT_NUMBER_FOR_ADVICE_W            0x0C14
#define PR_RECIPIENT_NUMBER_FOR_ADVICE_A            0x0C14
#define PR_RECIPIENT_TYPE                           0x0C15
#define PR_REGISTERED_MAIL_TYPE                     0x0C16
#define PR_REPLY_REQUESTED                          0x0C17
#define PR_REQUESTED_DELIVERY_METHOD                0x0C18
#define PR_SENDER_ENTRYID                           0x0C19
#define PR_SENDER_NAME                              0x0C1A
#define PR_SENDER_NAME_W                            0x0C1A
#define PR_SENDER_NAME_A                            0x0C1A
#define PR_SUPPLEMENTARY_INFO                       0x0C1B
#define PR_SUPPLEMENTARY_INFO_W                     0x0C1B
#define PR_SUPPLEMENTARY_INFO_A                     0x0C1B
#define PR_TYPE_OF_MTS_USER                         0x0C1C
#define PR_SENDER_SEARCH_KEY                        0x0C1D
#define PR_SENDER_ADDRTYPE                          0x0C1E
#define PR_SENDER_ADDRTYPE_W                        0x0C1E
#define PR_SENDER_ADDRTYPE_A                        0x0C1E
#define PR_SENDER_EMAIL_ADDRESS                     0x0C1F
#define PR_SENDER_EMAIL_ADDRESS_W                   0x0C1F
#define PR_SENDER_EMAIL_ADDRESS_A                   0x0C1F

/*
 *  Message non-transmittable properties
 */

#define PR_CURRENT_VERSION                          0x0E00
#define PR_DELETE_AFTER_SUBMIT                      0x0E01
#define PR_DISPLAY_BCC                              0x0E02
#define PR_DISPLAY_BCC_W                            0x0E02
#define PR_DISPLAY_BCC_A                            0x0E02
#define PR_DISPLAY_CC                               0x0E03
#define PR_DISPLAY_CC_W                             0x0E03
#define PR_DISPLAY_CC_A                             0x0E03
#define PR_DISPLAY_TO                               0x0E04
#define PR_DISPLAY_TO_W                             0x0E04
#define PR_DISPLAY_TO_A                             0x0E04
#define PR_PARENT_DISPLAY                           0x0E05
#define PR_PARENT_DISPLAY_W                         0x0E05
#define PR_PARENT_DISPLAY_A                         0x0E05
#define PR_MESSAGE_DELIVERY_TIME                    0x0E06
#define PR_MESSAGE_FLAGS                            0x0E07
#define PR_MESSAGE_SIZE                             0x0E08
#define PR_PARENT_ENTRYID                           0x0E09
#define PR_SENTMAIL_ENTRYID                         0x0E0A
#define PR_CORRELATE                                0x0E0C
#define PR_CORRELATE_MTSID                          0x0E0D
#define PR_DISCRETE_VALUES                          0x0E0E
#define PR_RESPONSIBILITY                           0x0E0F
#define PR_SPOOLER_STATUS                           0x0E10
#define PR_TRANSPORT_STATUS                         0x0E11
#define PR_MESSAGE_RECIPIENTS                       0x0E12
#define PR_MESSAGE_ATTACHMENTS                      0x0E13
#define PR_SUBMIT_FLAGS                             0x0E14
#define PR_RECIPIENT_STATUS                         0x0E15
#define PR_TRANSPORT_KEY                            0x0E16
#define PR_MSG_STATUS                               0x0E17
#define PR_MESSAGE_DOWNLOAD_TIME                    0x0E18
#define PR_CREATION_VERSION                         0x0E19
#define PR_MODIFY_VERSION                           0x0E1A
#define PR_HASATTACH                                0x0E1B
#define PR_BODY_CRC                                 0x0E1C
#define PR_NORMALIZED_SUBJECT                       0x0E1D
#define PR_NORMALIZED_SUBJECT_W                     0x0E1D
#define PR_NORMALIZED_SUBJECT_A                     0x0E1D
#define PR_RTF_IN_SYNC                              0x0E1F
#define PR_ATTACH_SIZE                              0x0E20
#define PR_ATTACH_NUM                               0x0E21
#define PR_PREPROCESS                               0x0E22

/* PR_ORIGINAL_DISPLAY_TO, _CC, and _BCC moved to transmittible range 03/09/95 */

#define PR_ORIGINATING_MTA_CERTIFICATE              0x0E25
#define PR_PROOF_OF_SUBMISSION                      0x0E26

/*
 *  Properties common to numerous MAPI objects.
 *
 *  Those properties that can appear on messages are in the
 *  non-transmittable range for messages. They start at the high
 *  end of that range and work down.
 *
 *  Properties that never appear on messages are defined in the common
 *  property range (see above).
 */

/*
 * properties that are common to multiple objects (including message objects
 * -- these ids are in the non-transmittable range
 */

#define PR_ENTRYID                                  0x0FFF
#define PR_OBJECT_TYPE                              0x0FFE
#define PR_ICON                                     0x0FFD
#define PR_MINI_ICON                                0x0FFC
#define PR_STORE_ENTRYID                            0x0FFB
#define PR_STORE_RECORD_KEY                         0x0FFA
#define PR_RECORD_KEY                               0x0FF9
#define PR_MAPPING_SIGNATURE                        0x0FF8
#define PR_ACCESS_LEVEL                             0x0FF7
#define PR_INSTANCE_KEY                             0x0FF6
#define PR_ROW_TYPE                                 0x0FF5
#define PR_ACCESS                                   0x0FF4

/*
 * properties that are common to multiple objects (usually not including message objects
 * -- these ids are in the transmittable range
 */

#define PR_ROWID                                    0x3000
#define PR_DISPLAY_NAME                             0x3001
#define PR_DISPLAY_NAME_W                           0x3001
#define PR_DISPLAY_NAME_A                           0x3001
#define PR_ADDRTYPE                                 0x3002
#define PR_ADDRTYPE_W                               0x3002
#define PR_ADDRTYPE_A                               0x3002
#define PR_EMAIL_ADDRESS                            0x3003
#define PR_EMAIL_ADDRESS_W                          0x3003
#define PR_EMAIL_ADDRESS_A                          0x3003
#define PR_COMMENT                                  0x3004
#define PR_COMMENT_W                                0x3004
#define PR_COMMENT_A                                0x3004
#define PR_DEPTH                                    0x3005
#define PR_PROVIDER_DISPLAY                         0x3006
#define PR_PROVIDER_DISPLAY_W                       0x3006
#define PR_PROVIDER_DISPLAY_A                       0x3006
#define PR_CREATION_TIME                            0x3007
#define PR_LAST_MODIFICATION_TIME                   0x3008
#define PR_RESOURCE_FLAGS                           0x3009
#define PR_PROVIDER_DLL_NAME                        0x300A
#define PR_PROVIDER_DLL_NAME_W                      0x300A
#define PR_PROVIDER_DLL_NAME_A                      0x300A
#define PR_SEARCH_KEY                               0x300B
#define PR_PROVIDER_UID                             0x300C
#define PR_PROVIDER_ORDINAL                         0x300D

/*
 *  MAPI Form properties
 */
#define PR_FORM_VERSION                             0x3301
#define PR_FORM_VERSION_W                           0x3301
#define PR_FORM_VERSION_A                           0x3301
#define PR_FORM_CLSID                               0x3302
#define PR_FORM_CONTACT_NAME                        0x3303
#define PR_FORM_CONTACT_NAME_W                      0x3303
#define PR_FORM_CONTACT_NAME_A                      0x3303
#define PR_FORM_CATEGORY                            0x3304
#define PR_FORM_CATEGORY_W                          0x3304
#define PR_FORM_CATEGORY_A                          0x3304
#define PR_FORM_CATEGORY_SUB                        0x3305
#define PR_FORM_CATEGORY_SUB_W                      0x3305
#define PR_FORM_CATEGORY_SUB_A                      0x3305
#define PR_FORM_HOST_MAP                            0x3306
#define PR_FORM_HIDDEN                              0x3307
#define PR_FORM_DESIGNER_NAME                       0x3308
#define PR_FORM_DESIGNER_NAME_W                     0x3308
#define PR_FORM_DESIGNER_NAME_A                     0x3308
#define PR_FORM_DESIGNER_GUID                       0x3309
#define PR_FORM_MESSAGE_BEHAVIOR                    0x330A

/*
 *  Message store properties
 */

#define PR_DEFAULT_STORE                            0x3400
#define PR_STORE_SUPPORT_MASK                       0x340D
#define PR_STORE_STATE                              0x340E

#define PR_IPM_SUBTREE_SEARCH_KEY                   0x3410
#define PR_IPM_OUTBOX_SEARCH_KEY                    0x3411
#define PR_IPM_WASTEBASKET_SEARCH_KEY               0x3412
#define PR_IPM_SENTMAIL_SEARCH_KEY                  0x3413
#define PR_MDB_PROVIDER                             0x3414
#define PR_RECEIVE_FOLDER_SETTINGS                  0x3415

#define PR_VALID_FOLDER_MASK                        0x35DF
#define PR_IPM_SUBTREE_ENTRYID                      0x35E0

#define PR_IPM_OUTBOX_ENTRYID                       0x35E2
#define PR_IPM_WASTEBASKET_ENTRYID                  0x35E3
#define PR_IPM_SENTMAIL_ENTRYID                     0x35E4
#define PR_VIEWS_ENTRYID                            0x35E5
#define PR_COMMON_VIEWS_ENTRYID                     0x35E6
#define PR_FINDER_ENTRYID                           0x35E7

/* Proptags 0x35E8-0x35FF reserved for folders "guaranteed" by PR_VALID_FOLDER_MASK */


/*
 *  Folder and AB Container properties
 */

#define PR_CONTAINER_FLAGS                          0x3600
#define PR_FOLDER_TYPE                              0x3601
#define PR_CONTENT_COUNT                            0x3602
#define PR_CONTENT_UNREAD                           0x3603
#define PR_CREATE_TEMPLATES                         0x3604
#define PR_DETAILS_TABLE                            0x3605
#define PR_SEARCH                                   0x3607
#define PR_SELECTABLE                               0x3609
#define PR_SUBFOLDERS                               0x360A
#define PR_STATUS                                   0x360B
#define PR_ANR                                      0x360C
#define PR_ANR_W                                    0x360C
#define PR_ANR_A                                    0x360C
#define PR_CONTENTS_SORT_ORDER                      0x360D
#define PR_CONTAINER_HIERARCHY                      0x360E
#define PR_CONTAINER_CONTENTS                       0x360F
#define PR_FOLDER_ASSOCIATED_CONTENTS               0x3610
#define PR_DEF_CREATE_DL                            0x3611
#define PR_DEF_CREATE_MAILUSER                      0x3612
#define PR_CONTAINER_CLASS                          0x3613
#define PR_CONTAINER_CLASS_W                        0x3613
#define PR_CONTAINER_CLASS_A                        0x3613
#define PR_CONTAINER_MODIFY_VERSION                 0x3614
#define PR_AB_PROVIDER_ID                           0x3615
#define PR_DEFAULT_VIEW_ENTRYID                     0x3616
#define PR_ASSOC_CONTENT_COUNT                      0x3617

/* Reserved 0x36C0-0x36FF */

/*
 *  Attachment properties
 */

#define PR_ATTACHMENT_X400_PARAMETERS               0x3700
#define PR_ATTACH_DATA_OBJ                          0x3701
#define PR_ATTACH_DATA_BIN                          0x3701
#define PR_ATTACH_ENCODING                          0x3702
#define PR_ATTACH_EXTENSION                         0x3703
#define PR_ATTACH_EXTENSION_W                       0x3703
#define PR_ATTACH_EXTENSION_A                       0x3703
#define PR_ATTACH_FILENAME                          0x3704
#define PR_ATTACH_FILENAME_W                        0x3704
#define PR_ATTACH_FILENAME_A                        0x3704
#define PR_ATTACH_METHOD                            0x3705
#define PR_ATTACH_LONG_FILENAME                     0x3707
#define PR_ATTACH_LONG_FILENAME_W                   0x3707
#define PR_ATTACH_LONG_FILENAME_A                   0x3707
#define PR_ATTACH_PATHNAME                          0x3708
#define PR_ATTACH_PATHNAME_W                        0x3708
#define PR_ATTACH_PATHNAME_A                        0x3708
#define PR_ATTACH_RENDERING                         0x3709
#define PR_ATTACH_TAG                               0x370A
#define PR_RENDERING_POSITION                       0x370B
#define PR_ATTACH_TRANSPORT_NAME                    0x370C
#define PR_ATTACH_TRANSPORT_NAME_W                  0x370C
#define PR_ATTACH_TRANSPORT_NAME_A                  0x370C
#define PR_ATTACH_LONG_PATHNAME                     0x370D
#define PR_ATTACH_LONG_PATHNAME_W                   0x370D
#define PR_ATTACH_LONG_PATHNAME_A                   0x370D
#define PR_ATTACH_MIME_TAG                          0x370E
#define PR_ATTACH_MIME_TAG_W                        0x370E
#define PR_ATTACH_MIME_TAG_A                        0x370E
#define PR_ATTACH_ADDITIONAL_INFO                   0x370F

/*
 *  AB Object properties
 */

#define PR_DISPLAY_TYPE                             0x3900
#define PR_TEMPLATEID                               0x3902
#define PR_PRIMARY_CAPABILITY                       0x3904


/*
 *  Mail user properties
 */
#define PR_7BIT_DISPLAY_NAME                        0x39FF
#define PR_ACCOUNT                                  0x3A00
#define PR_ACCOUNT_W                                0x3A00
#define PR_ACCOUNT_A                                0x3A00
#define PR_ALTERNATE_RECIPIENT                      0x3A01
#define PR_CALLBACK_TELEPHONE_NUMBER                0x3A02
#define PR_CALLBACK_TELEPHONE_NUMBER_W              0x3A02
#define PR_CALLBACK_TELEPHONE_NUMBER_A              0x3A02
#define PR_CONVERSION_PROHIBITED                    0x3A03
#define PR_DISCLOSE_RECIPIENTS                      0x3A04
#define PR_GENERATION                               0x3A05
#define PR_GENERATION_W                             0x3A05
#define PR_GENERATION_A                             0x3A05
#define PR_GIVEN_NAME                               0x3A06
#define PR_GIVEN_NAME_W                             0x3A06
#define PR_GIVEN_NAME_A                             0x3A06
#define PR_GOVERNMENT_ID_NUMBER                     0x3A07
#define PR_GOVERNMENT_ID_NUMBER_W                   0x3A07
#define PR_GOVERNMENT_ID_NUMBER_A                   0x3A07
#define PR_BUSINESS_TELEPHONE_NUMBER                0x3A08
#define PR_BUSINESS_TELEPHONE_NUMBER_W              0x3A08
#define PR_BUSINESS_TELEPHONE_NUMBER_A              0x3A08
#define PR_OFFICE_TELEPHONE_NUMBER                  PR_BUSINESS_TELEPHONE_NUMBER
#define PR_OFFICE_TELEPHONE_NUMBER_W                PR_BUSINESS_TELEPHONE_NUMBER_W
#define PR_OFFICE_TELEPHONE_NUMBER_A                PR_BUSINESS_TELEPHONE_NUMBER_A
#define PR_HOME_TELEPHONE_NUMBER                    0x3A09
#define PR_HOME_TELEPHONE_NUMBER_W                  0x3A09
#define PR_HOME_TELEPHONE_NUMBER_A                  0x3A09
#define PR_INITIALS                                 0x3A0A
#define PR_INITIALS_W                               0x3A0A
#define PR_INITIALS_A                               0x3A0A
#define PR_KEYWORD                                  0x3A0B
#define PR_KEYWORD_W                                0x3A0B
#define PR_KEYWORD_A                                0x3A0B
#define PR_LANGUAGE                                 0x3A0C
#define PR_LANGUAGE_W                               0x3A0C
#define PR_LANGUAGE_A                               0x3A0C
#define PR_LOCATION                                 0x3A0D
#define PR_LOCATION_W                               0x3A0D
#define PR_LOCATION_A                               0x3A0D
#define PR_MAIL_PERMISSION                          0x3A0E
#define PR_MHS_COMMON_NAME                          0x3A0F
#define PR_MHS_COMMON_NAME_W                        0x3A0F
#define PR_MHS_COMMON_NAME_A                        0x3A0F
#define PR_ORGANIZATIONAL_ID_NUMBER                 0x3A10
#define PR_ORGANIZATIONAL_ID_NUMBER_W               0x3A10
#define PR_ORGANIZATIONAL_ID_NUMBER_A               0x3A10
#define PR_SURNAME                                  0x3A11
#define PR_SURNAME_W                                0x3A11
#define PR_SURNAME_A                                0x3A11
#define PR_ORIGINAL_ENTRYID                         0x3A12
#define PR_ORIGINAL_DISPLAY_NAME                    0x3A13
#define PR_ORIGINAL_DISPLAY_NAME_W                  0x3A13
#define PR_ORIGINAL_DISPLAY_NAME_A                  0x3A13
#define PR_ORIGINAL_SEARCH_KEY                      0x3A14
#define PR_POSTAL_ADDRESS                           0x3A15
#define PR_POSTAL_ADDRESS_W                         0x3A15
#define PR_POSTAL_ADDRESS_A                         0x3A15
#define PR_COMPANY_NAME                             0x3A16
#define PR_COMPANY_NAME_W                           0x3A16
#define PR_COMPANY_NAME_A                           0x3A16
#define PR_TITLE                                    0x3A17
#define PR_TITLE_W                                  0x3A17
#define PR_TITLE_A                                  0x3A17
#define PR_DEPARTMENT_NAME                          0x3A18
#define PR_DEPARTMENT_NAME_W                        0x3A18
#define PR_DEPARTMENT_NAME_A                        0x3A18
#define PR_OFFICE_LOCATION                          0x3A19
#define PR_OFFICE_LOCATION_W                        0x3A19
#define PR_OFFICE_LOCATION_A                        0x3A19
#define PR_PRIMARY_TELEPHONE_NUMBER                 0x3A1A
#define PR_PRIMARY_TELEPHONE_NUMBER_W               0x3A1A
#define PR_PRIMARY_TELEPHONE_NUMBER_A               0x3A1A
#define PR_BUSINESS2_TELEPHONE_NUMBER               0x3A1B
#define PR_BUSINESS2_TELEPHONE_NUMBER_W             0x3A1B
#define PR_BUSINESS2_TELEPHONE_NUMBER_A             0x3A1B
#define PR_OFFICE2_TELEPHONE_NUMBER                 PR_BUSINESS2_TELEPHONE_NUMBER
#define PR_OFFICE2_TELEPHONE_NUMBER_W               PR_BUSINESS2_TELEPHONE_NUMBER_W
#define PR_OFFICE2_TELEPHONE_NUMBER_A               PR_BUSINESS2_TELEPHONE_NUMBER_A
#define PR_MOBILE_TELEPHONE_NUMBER                  0x3A1C
#define PR_MOBILE_TELEPHONE_NUMBER_W                0x3A1C
#define PR_MOBILE_TELEPHONE_NUMBER_A                0x3A1C
#define PR_CELLULAR_TELEPHONE_NUMBER                PR_MOBILE_TELEPHONE_NUMBER
#define PR_CELLULAR_TELEPHONE_NUMBER_W              PR_MOBILE_TELEPHONE_NUMBER_W
#define PR_CELLULAR_TELEPHONE_NUMBER_A              PR_MOBILE_TELEPHONE_NUMBER_A
#define PR_RADIO_TELEPHONE_NUMBER                   0x3A1D
#define PR_RADIO_TELEPHONE_NUMBER_W                 0x3A1D
#define PR_RADIO_TELEPHONE_NUMBER_A                 0x3A1D
#define PR_CAR_TELEPHONE_NUMBER                     0x3A1E
#define PR_CAR_TELEPHONE_NUMBER_W                   0x3A1E
#define PR_CAR_TELEPHONE_NUMBER_A                   0x3A1E
#define PR_OTHER_TELEPHONE_NUMBER                   0x3A1F
#define PR_OTHER_TELEPHONE_NUMBER_W                 0x3A1F
#define PR_OTHER_TELEPHONE_NUMBER_A                 0x3A1F
#define PR_TRANSMITABLE_DISPLAY_NAME                0x3A20
#define PR_TRANSMITABLE_DISPLAY_NAME_W              0x3A20
#define PR_TRANSMITABLE_DISPLAY_NAME_A              0x3A20
#define PR_PAGER_TELEPHONE_NUMBER                   0x3A21
#define PR_PAGER_TELEPHONE_NUMBER_W                 0x3A21
#define PR_PAGER_TELEPHONE_NUMBER_A                 0x3A21
#define PR_BEEPER_TELEPHONE_NUMBER                  PR_PAGER_TELEPHONE_NUMBER
#define PR_BEEPER_TELEPHONE_NUMBER_W                PR_PAGER_TELEPHONE_NUMBER_W
#define PR_BEEPER_TELEPHONE_NUMBER_A                PR_PAGER_TELEPHONE_NUMBER_A
#define PR_USER_CERTIFICATE                         0x3A22
#define PR_PRIMARY_FAX_NUMBER                       0x3A23
#define PR_PRIMARY_FAX_NUMBER_W                     0x3A23
#define PR_PRIMARY_FAX_NUMBER_A                     0x3A23
#define PR_BUSINESS_FAX_NUMBER                      0x3A24
#define PR_BUSINESS_FAX_NUMBER_W                    0x3A24
#define PR_BUSINESS_FAX_NUMBER_A                    0x3A24
#define PR_HOME_FAX_NUMBER                          0x3A25
#define PR_HOME_FAX_NUMBER_W                        0x3A25
#define PR_HOME_FAX_NUMBER_A                        0x3A25
#define PR_COUNTRY                                  0x3A26
#define PR_COUNTRY_W                                0x3A26
#define PR_COUNTRY_A                                0x3A26
#define PR_BUSINESS_ADDRESS_COUNTRY                 PR_COUNTRY
#define PR_BUSINESS_ADDRESS_COUNTRY_W               PR_COUNTRY_W
#define PR_BUSINESS_ADDRESS_COUNTRY_A               PR_COUNTRY_A

#define PR_LOCALITY                                 0x3A27
#define PR_LOCALITY_W                               0x3A27
#define PR_LOCALITY_A                               0x3A27
#define PR_BUSINESS_ADDRESS_CITY                    PR_LOCALITY
#define PR_BUSINESS_ADDRESS_CITY_W                  PR_LOCALITY_W
#define PR_BUSINESS_ADDRESS_CITY_A                  PR_LOCALITY_A

#define PR_STATE_OR_PROVINCE                        0x3A28
#define PR_STATE_OR_PROVINCE_W                      0x3A28
#define PR_STATE_OR_PROVINCE_A                      0x3A28
#define PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE       PR_STATE_OR_PROVINCE
#define PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE_W     PR_STATE_OR_PROVINCE_W
#define PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE_A     PR_STATE_OR_PROVINCE_A

#define PR_STREET_ADDRESS                           0x3A29
#define PR_STREET_ADDRESS_W                         0x3A29
#define PR_STREET_ADDRESS_A                         0x3A29
#define PR_BUSINESS_ADDRESS_STREET                  PR_STREET_ADDRESS
#define PR_BUSINESS_ADDRESS_STREET_W                PR_STREET_ADDRESS_W
#define PR_BUSINESS_ADDRESS_STREET_A                PR_STREET_ADDRESS_A

#define PR_POSTAL_CODE                              0x3A2A
#define PR_POSTAL_CODE_W                            0x3A2A
#define PR_POSTAL_CODE_A                            0x3A2A
#define PR_BUSINESS_ADDRESS_POSTAL_CODE             PR_POSTAL_CODE
#define PR_BUSINESS_ADDRESS_POSTAL_CODE_W           PR_POSTAL_CODE_W
#define PR_BUSINESS_ADDRESS_POSTAL_CODE_A           PR_POSTAL_CODE_A


#define PR_POST_OFFICE_BOX                          0x3A2B
#define PR_POST_OFFICE_BOX_W                        0x3A2B
#define PR_POST_OFFICE_BOX_A                        0x3A2B
#define PR_BUSINESS_ADDRESS_POST_OFFICE_BOX         PR_POST_OFFICE_BOX
#define PR_BUSINESS_ADDRESS_POST_OFFICE_BOX_W       PR_POST_OFFICE_BOX_W
#define PR_BUSINESS_ADDRESS_POST_OFFICE_BOX_A       PR_POST_OFFICE_BOX_A


#define PR_TELEX_NUMBER                             0x3A2C
#define PR_TELEX_NUMBER_W                           0x3A2C
#define PR_TELEX_NUMBER_A                           0x3A2C
#define PR_ISDN_NUMBER                              0x3A2D
#define PR_ISDN_NUMBER_W                            0x3A2D
#define PR_ISDN_NUMBER_A                            0x3A2D
#define PR_ASSISTANT_TELEPHONE_NUMBER               0x3A2E
#define PR_ASSISTANT_TELEPHONE_NUMBER_W             0x3A2E
#define PR_ASSISTANT_TELEPHONE_NUMBER_A             0x3A2E
#define PR_HOME2_TELEPHONE_NUMBER                   0x3A2F
#define PR_HOME2_TELEPHONE_NUMBER_W                 0x3A2F
#define PR_HOME2_TELEPHONE_NUMBER_A                 0x3A2F
#define PR_ASSISTANT                                0x3A30
#define PR_ASSISTANT_W                              0x3A30
#define PR_ASSISTANT_A                              0x3A30
#define PR_SEND_RICH_INFO                           0x3A40

#define PR_WEDDING_ANNIVERSARY                      0x3A41
#define PR_BIRTHDAY                                 0x3A42


#define PR_HOBBIES                                  0x3A43
#define PR_HOBBIES_W                                0x3A43
#define PR_HOBBIES_A                                0x3A43

#define PR_MIDDLE_NAME                              0x3A44
#define PR_MIDDLE_NAME_W                            0x3A44
#define PR_MIDDLE_NAME_A                            0x3A44

#define PR_DISPLAY_NAME_PREFIX                      0x3A45
#define PR_DISPLAY_NAME_PREFIX_W                    0x3A45
#define PR_DISPLAY_NAME_PREFIX_A                    0x3A45

#define PR_PROFESSION                               0x3A46
#define PR_PROFESSION_W                             0x3A46
#define PR_PROFESSION_A                             0x3A46

#define PR_PREFERRED_BY_NAME                        0x3A47
#define PR_PREFERRED_BY_NAME_W                      0x3A47
#define PR_PREFERRED_BY_NAME_A                      0x3A47

#define PR_SPOUSE_NAME                              0x3A48
#define PR_SPOUSE_NAME_W                            0x3A48
#define PR_SPOUSE_NAME_A                            0x3A48

#define PR_COMPUTER_NETWORK_NAME                    0x3A49
#define PR_COMPUTER_NETWORK_NAME_W                  0x3A49
#define PR_COMPUTER_NETWORK_NAME_A                  0x3A49

#define PR_CUSTOMER_ID                              0x3A4A
#define PR_CUSTOMER_ID_W                            0x3A4A
#define PR_CUSTOMER_ID_A                            0x3A4A

#define PR_TTYTDD_PHONE_NUMBER                      0x3A4B
#define PR_TTYTDD_PHONE_NUMBER_W                    0x3A4B
#define PR_TTYTDD_PHONE_NUMBER_A                    0x3A4B

#define PR_FTP_SITE                                 0x3A4C
#define PR_FTP_SITE_W                               0x3A4C
#define PR_FTP_SITE_A                               0x3A4C

#define PR_GENDER                                   0x3A4D

#define PR_MANAGER_NAME                             0x3A4E
#define PR_MANAGER_NAME_W                           0x3A4E
#define PR_MANAGER_NAME_A                           0x3A4E

#define PR_NICKNAME                                 0x3A4F
#define PR_NICKNAME_W                               0x3A4F
#define PR_NICKNAME_A                               0x3A4F

#define PR_PERSONAL_HOME_PAGE                       0x3A50
#define PR_PERSONAL_HOME_PAGE_W                     0x3A50
#define PR_PERSONAL_HOME_PAGE_A                     0x3A50


#define PR_BUSINESS_HOME_PAGE                       0x3A51
#define PR_BUSINESS_HOME_PAGE_W                     0x3A51
#define PR_BUSINESS_HOME_PAGE_A                     0x3A51

#define PR_CONTACT_VERSION                          0x3A52
#define PR_CONTACT_ENTRYIDS                         0x3A53

#define PR_CONTACT_ADDRTYPES                        0x3A54
#define PR_CONTACT_ADDRTYPES_W                      0x3A54
#define PR_CONTACT_ADDRTYPES_A                      0x3A54

#define PR_CONTACT_DEFAULT_ADDRESS_INDEX            0x3A55

#define PR_CONTACT_EMAIL_ADDRESSES                  0x3A56
#define PR_CONTACT_EMAIL_ADDRESSES_W                0x3A56
#define PR_CONTACT_EMAIL_ADDRESSES_A                0x3A56


#define PR_COMPANY_MAIN_PHONE_NUMBER                0x3A57
#define PR_COMPANY_MAIN_PHONE_NUMBER_W              0x3A57
#define PR_COMPANY_MAIN_PHONE_NUMBER_A              0x3A57

#define PR_CHILDRENS_NAMES                          0x3A58
#define PR_CHILDRENS_NAMES_W                        0x3A58
#define PR_CHILDRENS_NAMES_A                        0x3A58



#define PR_HOME_ADDRESS_CITY                        0x3A59
#define PR_HOME_ADDRESS_CITY_W                      0x3A59
#define PR_HOME_ADDRESS_CITY_A                      0x3A59

#define PR_HOME_ADDRESS_COUNTRY                     0x3A5A
#define PR_HOME_ADDRESS_COUNTRY_W                   0x3A5A
#define PR_HOME_ADDRESS_COUNTRY_A                   0x3A5A

#define PR_HOME_ADDRESS_POSTAL_CODE                 0x3A5B
#define PR_HOME_ADDRESS_POSTAL_CODE_W               0x3A5B
#define PR_HOME_ADDRESS_POSTAL_CODE_A               0x3A5B

#define PR_HOME_ADDRESS_STATE_OR_PROVINCE           0x3A5C
#define PR_HOME_ADDRESS_STATE_OR_PROVINCE_W         0x3A5C
#define PR_HOME_ADDRESS_STATE_OR_PROVINCE_A         0x3A5C

#define PR_HOME_ADDRESS_STREET                      0x3A5D
#define PR_HOME_ADDRESS_STREET_W                    0x3A5D
#define PR_HOME_ADDRESS_STREET_A                    0x3A5D

#define PR_HOME_ADDRESS_POST_OFFICE_BOX             0x3A5E
#define PR_HOME_ADDRESS_POST_OFFICE_BOX_W           0x3A5E
#define PR_HOME_ADDRESS_POST_OFFICE_BOX_A           0x3A5E

#define PR_OTHER_ADDRESS_CITY                       0x3A5F
#define PR_OTHER_ADDRESS_CITY_W                     0x3A5F
#define PR_OTHER_ADDRESS_CITY_A                     0x3A5F

#define PR_OTHER_ADDRESS_COUNTRY                    0x3A60
#define PR_OTHER_ADDRESS_COUNTRY_W                  0x3A60
#define PR_OTHER_ADDRESS_COUNTRY_A                  0x3A60

#define PR_OTHER_ADDRESS_POSTAL_CODE                0x3A61
#define PR_OTHER_ADDRESS_POSTAL_CODE_W              0x3A61
#define PR_OTHER_ADDRESS_POSTAL_CODE_A              0x3A61

#define PR_OTHER_ADDRESS_STATE_OR_PROVINCE          0x3A62
#define PR_OTHER_ADDRESS_STATE_OR_PROVINCE_W        0x3A62
#define PR_OTHER_ADDRESS_STATE_OR_PROVINCE_A        0x3A62

#define PR_OTHER_ADDRESS_STREET                     0x3A63
#define PR_OTHER_ADDRESS_STREET_W                   0x3A63
#define PR_OTHER_ADDRESS_STREET_A                   0x3A63

#define PR_OTHER_ADDRESS_POST_OFFICE_BOX            0x3A64
#define PR_OTHER_ADDRESS_POST_OFFICE_BOX_W          0x3A64
#define PR_OTHER_ADDRESS_POST_OFFICE_BOX_A          0x3A64


/*
 *  Profile section properties
 */

#define PR_STORE_PROVIDERS                          0x3D00
#define PR_AB_PROVIDERS                             0x3D01
#define PR_TRANSPORT_PROVIDERS                      0x3D02

#define PR_DEFAULT_PROFILE                          0x3D04
#define PR_AB_SEARCH_PATH                           0x3D05
#define PR_AB_DEFAULT_DIR                           0x3D06
#define PR_AB_DEFAULT_PAB                           0x3D07

#define PR_FILTERING_HOOKS                          0x3D08
#define PR_SERVICE_NAME                             0x3D09
#define PR_SERVICE_NAME_W                           0x3D09
#define PR_SERVICE_NAME_A                           0x3D09
#define PR_SERVICE_DLL_NAME                         0x3D0A
#define PR_SERVICE_DLL_NAME_W                       0x3D0A
#define PR_SERVICE_DLL_NAME_A                       0x3D0A
#define PR_SERVICE_ENTRY_NAME                       0x3D0B
#define PR_SERVICE_UID                              0x3D0C
#define PR_SERVICE_EXTRA_UIDS                       0x3D0D
#define PR_SERVICES                                 0x3D0E
#define PR_SERVICE_SUPPORT_FILES                    0x3D0F
#define PR_SERVICE_SUPPORT_FILES_W                  0x3D0F
#define PR_SERVICE_SUPPORT_FILES_A                  0x3D0F
#define PR_SERVICE_DELETE_FILES                     0x3D10
#define PR_SERVICE_DELETE_FILES_W                   0x3D10
#define PR_SERVICE_DELETE_FILES_A                   0x3D10
#define PR_AB_SEARCH_PATH_UPDATE                    0x3D11
#define PR_PROFILE_NAME                             0x3D12
#define PR_PROFILE_NAME_A                           0x3D12
#define PR_PROFILE_NAME_W                           0x3D12

/*
 *  Status object properties
 */

#define PR_IDENTITY_DISPLAY                         0x3E00
#define PR_IDENTITY_DISPLAY_W                       0x3E00
#define PR_IDENTITY_DISPLAY_A                       0x3E00
#define PR_IDENTITY_ENTRYID                         0x3E01
#define PR_RESOURCE_METHODS                         0x3E02
#define PR_RESOURCE_TYPE                            0x3E03
#define PR_STATUS_CODE                              0x3E04
#define PR_IDENTITY_SEARCH_KEY                      0x3E05
#define PR_OWN_STORE_ENTRYID                        0x3E06
#define PR_RESOURCE_PATH                            0x3E07
#define PR_RESOURCE_PATH_W                          0x3E07
#define PR_RESOURCE_PATH_A                          0x3E07
#define PR_STATUS_STRING                            0x3E08
#define PR_STATUS_STRING_W                          0x3E08
#define PR_STATUS_STRING_A                          0x3E08
#define PR_X400_DEFERRED_DELIVERY_CANCEL            0x3E09
#define PR_HEADER_FOLDER_ENTRYID                    0x3E0A
#define PR_REMOTE_PROGRESS                          0x3E0B
#define PR_REMOTE_PROGRESS_TEXT                     0x3E0C
#define PR_REMOTE_PROGRESS_TEXT_W                   0x3E0C
#define PR_REMOTE_PROGRESS_TEXT_A                   0x3E0C
#define PR_REMOTE_VALIDATE_OK                       0x3E0D

/*
 * Display table properties
 */

#define PR_CONTROL_FLAGS                            0x3F00
#define PR_CONTROL_STRUCTURE                        0x3F01
#define PR_CONTROL_TYPE                             0x3F02
#define PR_DELTAX                                   0x3F03
#define PR_DELTAY                                   0x3F04
#define PR_XPOS                                     0x3F05
#define PR_YPOS                                     0x3F06
#define PR_CONTROL_ID                               0x3F07
#define PR_INITIAL_DETAILS_PANE                     0x3F08

/*
 * Secure property id range
 */

#define PROP_ID_SECURE_MIN                          0x67F0
#define PROP_ID_SECURE_MAX                          0x67FF

/*
 * HP Openmail as reenginered from the X.400 .PAB file.
 */

#define HP_OPENMAIL_JOB                            0x672b
#define HP_OPENMAIL_COMPANY_NAME                   0x6728
#define HP_OPENMAIL_DEPARTMENT                     0x6729
#define HP_OPENMAIL_SUBDEP                         0x672b
#define HP_OPENMAIL_LOCATION_OF_WORK               0x672a


#endif  /* MAPITAGS_H */
