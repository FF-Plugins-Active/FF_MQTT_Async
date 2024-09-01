#pragma once

UENUM(BlueprintType)
enum class EMQTTQOS_Async : uint8
{
	QoS_0 = 0	UMETA(DisplayName = "QoS Level 0", ToolTip = "At most once"),
	QoS_1 = 1	UMETA(DisplayName = "QoS Level 1", ToolTip = "At least once"),
	QoS_2 = 2	UMETA(DisplayName = "QoS Level 2", ToolTip = "Exactly once"),
};
ENUM_CLASS_FLAGS(EMQTTQOS_Async)

UENUM(BlueprintType)
enum class EMQTTVERSION_Async : uint8
{
	Default	= 0		UMETA(DisplayName = "Default", ToolTip = "Use 3.1.1 then fall back to 3.1"),
	V3		= 3		UMETA(DisplayName = "MQTT Version 3"),
	V3_1	= 4		UMETA(DisplayName = "MQTT Version 3.1"),
	V_5		= 5		UMETA(DisplayName = "MQTT Version 5"),
};
ENUM_CLASS_FLAGS(EMQTTVERSION_Async)