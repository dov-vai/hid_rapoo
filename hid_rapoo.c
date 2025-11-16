#include <linux/module.h>
#include <linux/hid.h>
#include <linux/power_supply.h>
#include <linux/slab.h>

#define USB_VENDOR_ID_RAPOO                 0x24ae
#define USB_DEVICE_ID_RAPOO_VT3_MAX_GEN2    0x1417

#define RAPOO_BATTERY_REPORT_SIZE           13
#define RAPOO_BATTERY_CAPACITY_INDEX        8

struct rapoo_data {
    struct hid_device *hdev;
    struct power_supply *battery;
    int battery_capacity;
    const char* model_name;
};

static int rapoo_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
    // hid_get_drvdata will return NULL for interfaces we chose to ignore.
    struct rapoo_data *rdata = hid_get_drvdata(hdev);
    if (!rdata) {
        return 0;
    }

    if (size == RAPOO_BATTERY_REPORT_SIZE) {
        int new_capacity = data[RAPOO_BATTERY_CAPACITY_INDEX];

        if (rdata->battery_capacity != new_capacity) {
            rdata->battery_capacity = new_capacity;
            power_supply_changed(rdata->battery);
        }
    }
    return 0;
}

static int battery_capacity_to_level(int capacity)
{
    if (capacity > 75)
        return POWER_SUPPLY_CAPACITY_LEVEL_HIGH;
    if (capacity > 20)
        return POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
    if (capacity > 5)
        return POWER_SUPPLY_CAPACITY_LEVEL_LOW;
    return POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
}

static int rapoo_get_property(struct power_supply *psy, enum power_supply_property psp, union power_supply_propval *val)
{
    struct rapoo_data *rdata = power_supply_get_drvdata(psy);
    switch (psp) {
    case POWER_SUPPLY_PROP_CAPACITY:
        val->intval = rdata->battery_capacity;
        break;
    case POWER_SUPPLY_PROP_SCOPE:
        val->intval = POWER_SUPPLY_SCOPE_DEVICE;
        break;
    case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
        val->intval = battery_capacity_to_level(rdata->battery_capacity);
        break;
    case POWER_SUPPLY_PROP_MANUFACTURER:
		val->strval = "Rapoo";
		break;
    case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = rdata->model_name;
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static enum power_supply_property rapoo_psy_properties[] = {
    POWER_SUPPLY_PROP_CAPACITY,
    POWER_SUPPLY_PROP_SCOPE,
    POWER_SUPPLY_PROP_CAPACITY_LEVEL,
    POWER_SUPPLY_PROP_MANUFACTURER,
    POWER_SUPPLY_PROP_MODEL_NAME
};

static int rapoo_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    struct rapoo_data *rdata;
    struct power_supply_config psy_cfg = {};
    struct power_supply_desc *psy_desc;
    int ret;

    // Standard HID setup for all interfaces. This ensures the mouse/keyboard parts work.
    ret = hid_parse(hdev);
    if (ret) {
        hid_err(hdev, "parse failed\n");
        return ret;
    }

    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret) {
        hid_err(hdev, "hw start failed\n");
        return ret;
    }

    // Battery data is sent over the keyboard interface. So we ignore everything else.
    if (hdev->collection->usage != HID_GD_KEYBOARD) {
        hid_info(hdev, "Skipping non-keyboard interface %s\n", hdev->name);
        // We must return 0 to allow the generic mouse driver to claim the other interfaces.
        // We set drvdata to NULL so raw_event knows to ignore this interface.
        hid_set_drvdata(hdev, NULL);
        return 0;
    }

    hid_info(hdev, "Found keyboard interface. Setting up battery reporting.\n");

    rdata = devm_kzalloc(&hdev->dev, sizeof(*rdata), GFP_KERNEL);
    if (!rdata)
        return -ENOMEM;

    switch (id->product) {
        case USB_DEVICE_ID_RAPOO_VT3_MAX_GEN2:
            rdata->model_name = "VT3 MAX Gen-2";
            break;
        default:
            rdata->model_name = hdev->name;
            break;
    }

    hid_info(hdev, "Detected model: %s\n", rdata->model_name);

    hid_set_drvdata(hdev, rdata);
    rdata->hdev = hdev;
    rdata->battery_capacity = -1;

    psy_desc = devm_kzalloc(&hdev->dev, sizeof(*psy_desc), GFP_KERNEL);
    if (!psy_desc)
        return -ENOMEM;

    psy_desc->name = devm_kasprintf(&hdev->dev, GFP_KERNEL, "rapoo-%04x-%04x", id->vendor, id->product);
    psy_desc->type = POWER_SUPPLY_TYPE_BATTERY;
    psy_desc->properties = rapoo_psy_properties;
    psy_desc->num_properties = ARRAY_SIZE(rapoo_psy_properties);
    psy_desc->get_property = rapoo_get_property;

    psy_cfg.drv_data = rdata;

    // Use devm_power_supply_register for automatic cleanup on disconnect
    rdata->battery = devm_power_supply_register(&hdev->dev, psy_desc, &psy_cfg);
    if (IS_ERR(rdata->battery)) {
        hid_err(hdev, "Failed to register power_supply\n");
        return PTR_ERR(rdata->battery);
    }

    return 0;
}

static void rapoo_remove(struct hid_device *hdev)
{
    hid_hw_stop(hdev);
}

static const struct hid_device_id rapoo_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAPOO, USB_DEVICE_ID_RAPOO_VT3_MAX_GEN2) },
    { }
};
MODULE_DEVICE_TABLE(hid, rapoo_devices);

static struct hid_driver rapoo_driver = {
    .name        = "rapoo",
    .id_table    = rapoo_devices,
    .probe       = rapoo_probe,
    .remove      = rapoo_remove,
    .raw_event   = rapoo_raw_event,
};

module_hid_driver(rapoo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dov-vai");
MODULE_DESCRIPTION("HID driver for Rapoo devices");