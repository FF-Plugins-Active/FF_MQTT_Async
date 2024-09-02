// Fill out your copyright notice in the Description page of Project Settings.

#include "MQTT_Manager_Paho_Async.h"

// Sets default values
AMQTT_Manager_Paho_Async::AMQTT_Manager_Paho_Async()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void AMQTT_Manager_Paho_Async::BeginPlay()
{
	Super::BeginPlay();
}

// Called when the game end or when destroyed.
void AMQTT_Manager_Paho_Async::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (this->Client)
	{
		this->MQTT_Async_Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void AMQTT_Manager_Paho_Async::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FPahoClientParams_Async AMQTT_Manager_Paho_Async::GetClientParams()
{
	return this->Client_Params;
}

void AMQTT_Manager_Paho_Async::MQTT_Async_Destroy()
{
	if (!this->Client)
	{
		return;
	}
	
	if (MQTTAsync_isConnected(this->Client))
	{
		MQTTAsync_disconnectOptions DisconnectOptions = MQTTAsync_disconnectOptions_initializer;
		MQTTAsync_disconnect(this->Client, &DisconnectOptions);
	}

	MQTTAsync_destroy(&this->Client);
}

void AMQTT_Manager_Paho_Async::MQTT_Async_Init(FDelegate_Paho_Connection_Async DelegateConnection, FPahoClientParams_Async In_Params)
{
	FJsonObjectWrapper TempCode;
	TempCode.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho_Async");
	TempCode.JsonObject->SetStringField("FunctionName", "MQTT_Async_Init");
	TempCode.JsonObject->SetStringField("AdditionalInfo", "");

	if (this->Client)
	{
		TempCode.JsonObject->SetStringField("Description", "Client already initialized.");
		DelegateConnection.ExecuteIfBound(false, TempCode);
		return;
	}

	if (!In_Params.IsParamsValid())
	{
		TempCode.JsonObject->SetStringField("Description", "Address and/or client id should not be empty.");
		DelegateConnection.ExecuteIfBound(false, TempCode);
		return;
	}

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateConnection, TempCode, In_Params]()
		{
			FString Protocol;
			TArray<FString> URL_Sections = UKismetStringLibrary::ParseIntoArray(In_Params.Address, "://");

			if (URL_Sections.Num() > 1)
			{
				Protocol = URL_Sections[0];
				this->SetSSLParams(Protocol, In_Params);
			}

			int RetVal = -1;
			MQTTAsync TempClient = nullptr;

			if (In_Params.Version == EMQTTVERSION_Async::V_5)
			{
				MQTTAsync_createOptions createOpts = MQTTAsync_createOptions_initializer;
				createOpts.MQTTVersion = MQTTVERSION_5;
				
				RetVal = MQTTAsync_createWithOptions(&TempClient, TCHAR_TO_UTF8(*In_Params.Address), TCHAR_TO_UTF8(*In_Params.ClientId), MQTTCLIENT_PERSISTENCE_NONE, NULL, &createOpts);

				if (Protocol == "wss" || Protocol == "ws")
				{
					this->Connection_Options = MQTTAsync_connectOptions_initializer5_ws;
				}

				else
				{
					this->Connection_Options = MQTTAsync_connectOptions_initializer5;
				}

				this->Connection_Options.cleanstart = 1;
			}

			else
			{
				RetVal = MQTTAsync_create(&TempClient, TCHAR_TO_UTF8(*In_Params.Address), TCHAR_TO_UTF8(*In_Params.ClientId), MQTTCLIENT_PERSISTENCE_NONE, NULL);

				if (Protocol == "wss" || Protocol == "ws")
				{
					this->Connection_Options = MQTTAsync_connectOptions_initializer_ws;
				}

				else
				{
					this->Connection_Options = MQTTAsync_connectOptions_initializer;
				}
			}

			this->Connection_Options.cleansession = 1;
			this->Connection_Options.keepAliveInterval = In_Params.KeepAliveInterval;
			this->Connection_Options.username = TCHAR_TO_UTF8(*In_Params.UserName);
			this->Connection_Options.password = TCHAR_TO_UTF8(*In_Params.Password);
			this->Connection_Options.MQTTVersion = (int32)In_Params.Version;
			this->Connection_Options.ssl = &this->SSL_Options;
			this->Connection_Options.context = this;
			this->Connection_Options.onSuccess = onConnect;
			this->Connection_Options.onSuccess5 = onConnect;
			this->Connection_Options.onFailure = onConnectFailure;
			this->Connection_Options.onFailure5 = onConnectFailure;

			if (RetVal != MQTTASYNC_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("Description", "There was a problem while creating client.");
				TempCode.JsonObject->SetNumberField("ErrorCode", RetVal);

				MQTTAsync_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			RetVal = MQTTAsync_setCallbacks(TempClient, this, AMQTT_Manager_Paho_Async::ConnectionLost, AMQTT_Manager_Paho_Async::MessageArrived, AMQTT_Manager_Paho_Async::DeliveryCompleted);

			if (RetVal != MQTTASYNC_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("Description", "There was a problem while setting callbacks.");
				TempCode.JsonObject->SetNumberField("ErrorCode", RetVal);

				MQTTAsync_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			RetVal = MQTTAsync_connect(TempClient, &this->Connection_Options);

			if (RetVal != MQTTASYNC_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("Description", "There was a problem while making connection.");
				TempCode.JsonObject->SetNumberField("ErrorCode", RetVal);
				
				MQTTAsync_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			AsyncTask(ENamedThreads::GameThread, [this, DelegateConnection, TempCode, TempClient, In_Params]()
				{
					this->Client = TempClient;
					this->Client_Params = In_Params;
					TempCode.JsonObject->SetStringField("Description", "Connection successful.");

					DelegateConnection.ExecuteIfBound(true, TempCode);
				}
			);
		}
	);
}

bool AMQTT_Manager_Paho_Async::MQTT_Async_Publish(FJsonObjectWrapper& Out_Code, FString In_Topic, FString In_Payload, EMQTTQOS_Async In_QoS, int32 In_Retained)
{
	Out_Code.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho_Async");
	Out_Code.JsonObject->SetStringField("FunctionName", "MQTT_Async_Publish");
	Out_Code.JsonObject->SetStringField("AdditionalInfo", "");

	if (!this->Client)
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not valid.");
		return false;
	}

	if (!MQTTAsync_isConnected(this->Client))
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not connected.");
		return false;
	}

	const int32 QoS = FMath::Clamp((int32)In_QoS, 0, 2);
	
	MQTTAsync_message PublishedMessage = MQTTAsync_message_initializer;
	PublishedMessage.payload = TCHAR_TO_UTF8(*In_Topic);
	PublishedMessage.payloadlen = In_Payload.Len();
	PublishedMessage.qos = QoS;
	PublishedMessage.retained = In_Retained;
	
	MQTTAsync_responseOptions ResponseOptions = MQTTAsync_responseOptions_initializer;
	ResponseOptions.context = this;
	ResponseOptions.onSuccess = onSend;
	ResponseOptions.onSuccess5 = onSend;
	ResponseOptions.onFailure = onSendFailure;
	ResponseOptions.onFailure5 = onSendFailure;
	
	const int RetVal = MQTTAsync_sendMessage(this->Client, TCHAR_TO_UTF8(*In_Topic), &PublishedMessage, &ResponseOptions);
	
	const FString DescSring = RetVal == MQTTASYNC_SUCCESS ? "Payload successfully published." : "There was a problem while publishing payload with these configurations.";
	Out_Code.JsonObject->SetNumberField("ErrorCode", RetVal);
	Out_Code.JsonObject->SetStringField("Description", DescSring);

	return RetVal == MQTTASYNC_SUCCESS ? true : false;
}

bool AMQTT_Manager_Paho_Async::MQTT_Async_Subscribe(FJsonObjectWrapper& Out_Code, FString In_Topic, EMQTTQOS_Async In_QoS)
{
	return false;
}