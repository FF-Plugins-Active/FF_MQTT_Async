#pragma once

// UE Includes.
#include "JsonObjectWrapper.h"
#include "JsonUtilities.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

// Custom Includes.
#include "MQTT_Async_Enums.h"

THIRD_PARTY_INCLUDES_START

#ifdef _WIN64
#include "Windows/AllowWindowsPlatformTypes.h"
#include "MQTTAsync.h"
#include "Windows/HideWindowsPlatformTypes.h"

#else
#include "MQTTAsync.h"
#endif

THIRD_PARTY_INCLUDES_END

#include "MQTT_Async_Includes.generated.h"

USTRUCT(BlueprintType)
struct FF_MQTT_ASYNC_API FPahoArrived_Async
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	FString Message;

	UPROPERTY(BlueprintReadOnly)
	FString TopicName;

	UPROPERTY(BlueprintReadOnly)
	int32 TopicLenght = 0;

	bool operator == (const FPahoArrived_Async& Other) const
	{
		return Message == Other.Message && TopicName == Other.TopicName && TopicLenght == Other.TopicLenght;
	}

	bool operator != (const FPahoArrived_Async& Other) const
	{
		return !(*this == Other);
	}
};

FORCEINLINE uint32 GetTypeHash(const FPahoArrived_Async& Key)
{
	uint32 Hash_Message = GetTypeHash(Key.Message);
	uint32 Hash_TopicName = GetTypeHash(Key.TopicName);
	uint32 Hash_TopicLenght = GetTypeHash(Key.TopicLenght);

	uint32 GenericHash;
	FMemory::Memset(&GenericHash, 0, sizeof(uint32));
	GenericHash = HashCombine(GenericHash, Hash_Message);
	GenericHash = HashCombine(GenericHash, Hash_TopicName);
	GenericHash = HashCombine(GenericHash, Hash_TopicLenght);

	return GenericHash;
}

USTRUCT(BlueprintType)
struct FF_MQTT_ASYNC_API FPahoClientParams_Async
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FString ClientId;

	UPROPERTY(BlueprintReadWrite)
	FString Address;

	UPROPERTY(BlueprintReadWrite)
	FString UserName;

	UPROPERTY(BlueprintReadWrite)
	FString Password;

	UPROPERTY(BlueprintReadWrite, AdvancedDisplay)
	FString CAPath;

	UPROPERTY(BlueprintReadWrite, AdvancedDisplay)
	FString Path_KeyStore;

	UPROPERTY(BlueprintReadWrite, AdvancedDisplay)
	FString Path_TrustStore;

	UPROPERTY(BlueprintReadWrite, AdvancedDisplay)
	FString Path_PrivateKey;

	UPROPERTY(BlueprintReadWrite, AdvancedDisplay)
	FString PrivateKeyPass;

	UPROPERTY(BlueprintReadWrite, AdvancedDisplay)
	FString CipherSuites;

	UPROPERTY(BlueprintReadWrite)
	int32 KeepAliveInterval = 20;

	UPROPERTY(BlueprintReadWrite)
	EMQTTVERSION_Async Version = EMQTTVERSION_Async::Default;

	bool IsParamsValid()
	{
		if (this->ClientId.IsEmpty() || this->Address.IsEmpty())
		{
			return false;
		}

		else
		{
			return true;
		}
	};

	bool operator == (const FPahoClientParams_Async& Other) const
	{
		return ClientId == Other.ClientId && Address == Other.Address && UserName == Other.UserName && Password == Other.Password && CAPath == Other.CAPath
			&& Path_KeyStore == Other.Path_KeyStore && Path_TrustStore == Other.Path_TrustStore && Path_PrivateKey == Other.Path_PrivateKey && PrivateKeyPass == Other.PrivateKeyPass
			&& CipherSuites == Other.CipherSuites && KeepAliveInterval == Other.KeepAliveInterval && Version == Other.Version;
	}

	bool operator != (const FPahoClientParams_Async& Other) const
	{
		return !(*this == Other);
	}
};

FORCEINLINE uint32 GetTypeHash(const FPahoClientParams_Async& Key)
{
	uint32 Hash_ClientId = GetTypeHash(Key.ClientId);
	uint32 Hash_Address = GetTypeHash(Key.Address);
	uint32 Hash_Username = GetTypeHash(Key.UserName);
	uint32 Hash_Password = GetTypeHash(Key.Password);
	uint32 Hash_CAPath = GetTypeHash(Key.CAPath);
	uint32 Hash_KeyStore = GetTypeHash(Key.Path_KeyStore);
	uint32 Hash_TrustStore = GetTypeHash(Key.Path_TrustStore);
	uint32 Hash_PrivateKey = GetTypeHash(Key.Path_PrivateKey);
	uint32 Hash_PrivateKeyPass = GetTypeHash(Key.PrivateKeyPass);
	uint32 Hash_Cipher = GetTypeHash(Key.CipherSuites);
	uint32 Hash_Interval = GetTypeHash(Key.KeepAliveInterval);
	uint32 Hash_Version = GetTypeHash(Key.Version);

	uint32 GenericHash;
	FMemory::Memset(&GenericHash, 0, sizeof(uint32));
	GenericHash = HashCombine(GenericHash, Hash_ClientId);
	GenericHash = HashCombine(GenericHash, Hash_Address);
	GenericHash = HashCombine(GenericHash, Hash_Username);
	GenericHash = HashCombine(GenericHash, Hash_Password);
	GenericHash = HashCombine(GenericHash, Hash_CAPath);
	GenericHash = HashCombine(GenericHash, Hash_KeyStore);
	GenericHash = HashCombine(GenericHash, Hash_TrustStore);
	GenericHash = HashCombine(GenericHash, Hash_PrivateKey);
	GenericHash = HashCombine(GenericHash, Hash_PrivateKeyPass);
	GenericHash = HashCombine(GenericHash, Hash_Cipher);
	GenericHash = HashCombine(GenericHash, Hash_Interval);
	GenericHash = HashCombine(GenericHash, Hash_Version);

	return GenericHash;
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegate_Paho_Delivered_Async, FString, Out_Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegate_Paho_Arrived_Async, FPahoArrived_Async, Out_Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegate_Paho_Lost_Async, FString, Out_Cause);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegate_Paho_Connection_Async, bool, bIsSuccessfull, FJsonObjectWrapper, Out_Code);