/**
 * Most of this is modified example code from ST.
 */

#include "ble.h"

#include <stdlib.h>

#include "bluenrg1_hci_le.h"
#include "bluenrg1_events.h"
#include "hci_tl.h"
#include "hci.h"
#include "bluenrg_utils.h"

#define PERIPHERAL_PASS_KEY (123456)

/* Private macros ------------------------------------------------------------*/
#define SENSOR_DEMO_NAME   'B','a','n','a','n','a','s'
#define BDADDR_SIZE        6

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

/* Private variables ---------------------------------------------------------*/
volatile uint8_t  set_connectable = 1;
volatile uint16_t connection_handle = 0;
volatile uint8_t  connected = FALSE;
volatile uint8_t  pairing = FALSE;
volatile uint8_t  paired = FALSE;
uint8_t bdaddr[BDADDR_SIZE];

uint16_t service_hndl, txchar_hndl, rxchar_hndl;
Service_UUID_t service_uuid;
Char_UUID_t char_uuid;

/* Private function prototypes -----------------------------------------------*/
static uint8_t DeviceInit(void);
void Set_DeviceConnectable(void);
void APP_UserEvtRx(void *pData);
tBleStatus Add_Service();

void MX_BlueNRG_2_Init(void) {
    hci_init(APP_UserEvtRx, NULL);
    DeviceInit();
}

void MX_BlueNRG_2_Process(void) {
    hci_user_evt_proc();
    
    /* Make the device discoverable */
    if (set_connectable)
    {
        Set_DeviceConnectable();
        set_connectable = FALSE;
    }

    if ((connected) && (!pairing))
    {
        aci_gap_slave_security_req(connection_handle);
        pairing = TRUE;
    }
}

uint8_t DeviceInit(void) {
    uint8_t ret;
    uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
    uint8_t device_name[] = {SENSOR_DEMO_NAME};
    uint8_t bdaddr_len_out;
    uint8_t config_data_stored_static_random_address = 0x80; /* Offset of the static random address stored in NVM */

    // SW reset
    hci_reset();
    
    // 2s delay required by ST
    HAL_Delay(2000);

    // Example code stuff to keep a static consistent address
    ret = aci_hal_read_config_data(config_data_stored_static_random_address,
                                   &bdaddr_len_out, bdaddr);
    if ((bdaddr[5] & 0xC0) != 0xC0) {
        // Static Random address not well formed
        while (1);
    }
    ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                                    bdaddr_len_out,
                                    bdaddr);

    /**
     * https://community.st.com/t5/interface-and-connectivity-ics/transmit-power-for-aci-hal-set-tx-power-level-for-bluenrg-2/td-p/210968
     * Please always set En_High_Power to 1.
     * And the below table for PA_Level,
     * 0: -14 dBm (High Power)
     * 1: -11 dBm (High Power)
     * 2: -8 dBm (High Power)
     * 3: -5 dBm (High Power)
     * 4: -2 dBm (High Power)
     * 5: 2 dBm (High Power)
     * 6: 4 dBm (High Power)
     * 7: 8 dBm (High Power) (default on reset)
     */
    ret = aci_hal_set_tx_power_level(1, 7);

    // GAP and GATT init
    ret = aci_gatt_init();
    ret = aci_gap_init(GAP_PERIPHERAL_ROLE, 0x00, 0x07, &service_handle, &dev_name_char_handle,
                       &appearance_char_handle);

    // Update device name
    ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, sizeof(device_name),
                                     device_name);
    
    // Security stuff: Confuses me. leaving sample code as is
    /*
     * Clear security database: this implies that each time the application is executed
     * the full bonding process is executed (with PassKey generation and setting).
     */
    ret = aci_gap_clear_security_db();

    /*
     * Set the I/O capability otherwise the Central device (e.g. the smartphone) will
     * propose a PIN that will be accepted without any control.
     */
    ret = aci_gap_set_io_capability(IO_CAP_DISPLAY_ONLY);

    /* BLE Security v4.2 is supported: BLE stack FW version >= 2.x (new API prototype) */
    ret = aci_gap_set_authentication_requirement(BONDING,
                                                 MITM_PROTECTION_REQUIRED,
                                                 SC_IS_SUPPORTED,
                                                 KEYPRESS_IS_NOT_SUPPORTED,
                                                 7,
                                                 16,
                                                 DONOT_USE_FIXED_PIN_FOR_PAIRING,
                                                 PERIPHERAL_PASS_KEY,
                                                 0x00); /* - 0x00: Public Identity Address
                                                           - 0x01: Random (static) Identity Address */

    // set up services I guess lol
    ret = Add_Service();

    // make compiler not say ret is unused...
    return ret;
}

// Puts the device in connectable mode
void Set_DeviceConnectable(void)
{
    uint8_t ret;
    uint8_t local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,SENSOR_DEMO_NAME};

    uint8_t manuf_data[26] = {
        2,0x0A,0x00, /* 0 dBm */  // Transmission Power
        8,0x09,SENSOR_DEMO_NAME,  // Complete Name
        13,0xFF,0x01, /* SKD version */
        0x80,
        0x00,
        0xF4, /* ACC+Gyro+Mag 0xE0 | 0x04 Temp | 0x10 Pressure */
        0x00, /*  */
        0x00, /*  */
        bdaddr[5], /* BLE MAC start -MSB first- */
        bdaddr[4],
        bdaddr[3],
        bdaddr[2],
        bdaddr[1],
        bdaddr[0]  /* BLE MAC stop */
    };

    manuf_data[18] |= 0x01; /* Sensor Fusion */

    hci_le_set_scan_response_data(0,NULL);

    PRINT_DBG("Set General Discoverable Mode.\r\n");

    ret = aci_gap_set_discoverable(ADV_DATA_TYPE,
                                   ADV_INTERV_MIN, ADV_INTERV_MAX,
                                   PUBLIC_ADDR,
                                   NO_WHITE_LIST_USE,
                                   sizeof(local_name), local_name, 0, NULL, 0, 0);

    aci_gap_update_adv_data(26, manuf_data);

    if(ret != BLE_STATUS_SUCCESS) {
        PRINT_DBG("aci_gap_set_discoverable() failed: 0x%02x\r\n", ret);
    } else {
        PRINT_DBG("aci_gap_set_discoverable() --> SUCCESS\r\n");
    }
}


/**
 * @brief  Callback processing the ACI events
 * @note   Inside this function each event must be identified and correctly
 *         parsed
 *         I'm going to just treat this as boilerplate and not touch it
 * @param  void* Pointer to the ACI packet
 */
void APP_UserEvtRx(void *pData)
{
    uint32_t i;

    hci_spi_pckt *hci_pckt = (hci_spi_pckt *)pData;

    if(hci_pckt->type == HCI_EVENT_PKT) {
        hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;

        if(event_pckt->evt == EVT_LE_META_EVENT) {
            evt_le_meta_event *evt = (void *)event_pckt->data;

            for (i = 0; i < (sizeof(hci_le_meta_events_table)/sizeof(hci_le_meta_events_table_type)); i++) {
                if (evt->subevent == hci_le_meta_events_table[i].evt_code) {
                    hci_le_meta_events_table[i].process((void *)evt->data);
                }
            }
        } else if(event_pckt->evt == EVT_VENDOR) {
            evt_blue_aci *blue_evt = (void*)event_pckt->data;

            for (i = 0; i < (sizeof(hci_vendor_specific_events_table)/sizeof(hci_vendor_specific_events_table_type)); i++) {
                if (blue_evt->ecode == hci_vendor_specific_events_table[i].evt_code) {
                    hci_vendor_specific_events_table[i].process((void *)blue_evt->data);
                }
            }
        } else {
            for (i = 0; i < (sizeof(hci_events_table)/sizeof(hci_events_table_type)); i++) {
                if (event_pckt->evt == hci_events_table[i].evt_code) {
                    hci_events_table[i].process((void *)event_pckt->data);
                }
            }
        }
    }
}

tBleStatus Add_Service() {
    tBleStatus ret;
    uint8_t uuid[16];

    uint8_t char_number = 2;
    uint8_t max_attribute_records = 1+(3*char_number); // service + chars*(declaration + value + descriptor)

    // https://www.uuidgenerator.net/
    // randomly generated uuids

    // add service
    // bc15926b-d435-4813-b232-c56830302c2d
    COPY_UUID_128(uuid,0xbc,0x15,0x92,0x6b,0xd4,0x35,0x48,0x13,0xb2,0x32,0xc5,0x68,0x30,0x30,0x2c,0x2d);
    BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
    ret = aci_gatt_add_service(UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE,
                               max_attribute_records, &service_hndl);
                                
    if (ret != BLE_STATUS_SUCCESS) {
        return ret;
    }

    // add characteristic for setting motor velos
    // 45d47eb2-7bf6-48f1-a4ff-08e14c92224c
    COPY_UUID_128(uuid,0x45,0x44,0x7e,0xb2,0x7b,0xf6,0x48,0xf1,0xa4,0xff,0x08,0xe1,0x4c,0x92,0x22,0x4c);
    BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
    ret = aci_gatt_add_char(service_hndl, UUID_TYPE_128, &char_uuid,
                            8, // 2 floats
                            CHAR_PROP_WRITE,
                            ATTR_PERMISSION_NONE,
                            GATT_NOTIFY_ATTRIBUTE_WRITE,
                            16, 0, &rxchar_hndl); 
    
    if (ret != BLE_STATUS_SUCCESS) {
        return ret;
    }

    // add characteristic for retrieving localization data
    // 6e18d67b-5fd7-4571-828e-fa04a317f5e3
    COPY_UUID_128(uuid,0x6e,0x18,0xd6,0x7b,0x5f,0xd7,0x45,0x71,0x82,0x8e,0xfa,0x04,0xa3,0x17,0xf5,0xe3);
    BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
    ret = aci_gatt_add_char(service_hndl, UUID_TYPE_128, &char_uuid,
                            16, // 4 floats, x, y, heading, spare
                            CHAR_PROP_READ,
                            ATTR_PERMISSION_NONE,
                            GATT_DONT_NOTIFY_EVENTS,
                            16, 0, &txchar_hndl);

    return ret;
}

/* ***************** BlueNRG-1 Stack Callbacks ********************************/
// new connection has been created
void hci_le_connection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)
{
    connected = TRUE;
    pairing = TRUE;
    paired = TRUE;

    connection_handle = Connection_Handle;
}

// connection is terminated
void hci_disconnection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Reason)
{
    connected = FALSE;
    pairing = FALSE;
    paired = FALSE;

    /* Make the device connectable again */
    set_connectable = TRUE;
    connection_handle = 0;
}

/**
 * This event is generated by the Security manager to the application
 * when a passkey is required for pairing.
 * When this event is received, the application has to respond with the
 * aci_gap_pass_key_resp command.
 */
void aci_gap_pass_key_req_event(uint16_t Connection_Handle) {
    aci_gap_pass_key_resp(connection_handle, PERIPHERAL_PASS_KEY);
}

/**
 * This event is generated when the pairing process has completed successfully or a pairing
 * procedure timeout has occurred or the pairing has failed. This is to notify the application that
 * we have paired with a remote device so that it can take further actions or to notify that a
 * timeout has occurred so that the upper layer can decide to disconnect the link.
 */
void aci_gap_pairing_complete_event(uint16_t connection_handle, uint8_t status, uint8_t reason) {
    if (status != 0x02) {
        paired = TRUE;
    }
}