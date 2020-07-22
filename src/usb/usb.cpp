#include "usb.h"

#include "usb_log_module.ii"

#include "nrf_drv_clock.h"

namespace Usb
{
	namespace Device
	{
		namespace
		{
			bool initialized = false;
			bool enabled = false;
			bool connected = false;
			bool supportsPowerDetection = false;
			map<const app_usbd_class_inst_t*, const class_info_t*> registry;
			void UsbdEventHandler(app_usbd_event_type_t event);
			static const app_usbd_config_t usbd_config = {
				.ev_state_proc = UsbdEventHandler
			};

		} // namespace

		bool Initialize(bool _supportsPowerDetection /*= true*/)
		{
			if (initialized)
			{
				return true;
			}

			ret_code_t ret = app_usbd_init(&usbd_config);
    		APP_ERROR_CHECK(ret);

			if (ret != NRF_SUCCESS)
			{
				return false;
			}

			supportsPowerDetection = _supportsPowerDetection;

			NRF_LOG_INFO("USB initialized. (Power Detction support: %s.)", supportsPowerDetection ? "enabled" : "disabled");

			initialized = true;
			return true;
		}

		bool IsInitialized()
		{
			return initialized;
		}

		bool Enable()
		{
			if (enabled)
			{
				return true;
			}

			if (supportsPowerDetection)
			{
				ret_code_t ret = app_usbd_power_events_enable();
				APP_ERROR_CHECK(ret);

				if (ret != NRF_SUCCESS)
				{
					NRF_LOG_ERROR("Unable to enable power events.");
					return false;
				}
				else
				{
					NRF_LOG_INFO("USB power events enabled.");
				}
			}
			else
			{
				NRF_LOG_INFO("No USB power detection enabled\r\nStarting USB now");

				app_usbd_enable();
				app_usbd_start();
				connected = true;

				NRF_LOG_INFO("USB enabled.");
			}

			return true;
		}

		bool Disable()
		{
			if (enabled == false)
			{
				return true;
			}

			app_usbd_stop();
			app_usbd_disable();

			NRF_LOG_INFO("USB disabled.");

			enabled = false;
			return true;
		}

		bool IsEnabled()
		{
			return enabled;
		}

		bool RegisterDeviceClass(const class_info_t* deviceClass)
		{
			if (deviceClass == nullptr)
			{
				NRF_LOG_ERROR("deviceClass cannot be nullptr.");
				return false;
			}

			if (registry.find(deviceClass->classInstance) != registry.end())
			{
				NRF_LOG_WARNING("Failed to register new device class, that class is already registered.");
				return false;
			}

			ret_code_t ret = app_usbd_class_append(deviceClass->classInstance);
			APP_ERROR_CHECK(ret);

			if (ret == NRF_SUCCESS)
			{
				registry[deviceClass->classInstance] = deviceClass;
				NRF_LOG_INFO("Registered USB device class `%s`.", deviceClass->name);
				return true;
			}

			NRF_LOG_WARNING("Failed to registered USB device class `%s`.", deviceClass->name);
			return false;
		}

		void Update()
		{
			while (app_usbd_event_queue_process())
			{
				/* Nothing to do */
			}
		}

		///
		/// File-local functions follow.
		///

		namespace
		{
			void UsbdEventHandler(app_usbd_event_type_t event)
			{
				switch (event)
				{
					case APP_USBD_EVT_DRV_SUSPEND:
						break;
					case APP_USBD_EVT_DRV_RESUME:
						break;
					case APP_USBD_EVT_STARTED:
						break;
					case APP_USBD_EVT_STOPPED:
						//UNUSED_RETURN_VALUE(fatfs_init()); #FSFS
						app_usbd_disable();
						break;
					case APP_USBD_EVT_POWER_DETECTED:
						NRF_LOG_INFO("USB power detected");

						if (!nrf_drv_usbd_is_enabled())
						{
							//fatfs_uninit(); #FSFS
							app_usbd_enable();
						}
						break;
					case APP_USBD_EVT_POWER_REMOVED:
						NRF_LOG_INFO("USB power removed");
						app_usbd_stop();
						connected = false;
						break;
					case APP_USBD_EVT_POWER_READY:
						NRF_LOG_INFO("USB ready");
						app_usbd_start();
						connected = true;
						break;
					default:
						break;
				}
			}
		}

	} // namespace Device
} // namespace Usb