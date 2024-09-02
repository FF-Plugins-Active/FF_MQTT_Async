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

#pragma region CALLBACKS

void AMQTT_Manager_Paho_Async::DeliveryCompleted(void* CallbackContext, MQTTAsync_token DeliveredToken)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!Owner)
	{
		return;
	}

	Owner->Delegate_Delivered.Broadcast(FString::FromInt(DeliveredToken));
}

int AMQTT_Manager_Paho_Async::MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTAsync_message* Message)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!Owner)
	{
		return 0;
	}

	FPahoArrived_Async StrArrived;
	StrArrived.TopicName = UTF8_TO_TCHAR(TopicName);
	StrArrived.TopicLenght = TopicLenght;
	StrArrived.Message.AppendChars(UTF8_TO_TCHAR((char*)Message->payload), Message->payloadlen);

	Owner->Delegate_Arrived.Broadcast(StrArrived);

	MQTTAsync_freeMessage(&Message);
	MQTTAsync_free(TopicName);

	return 1;
}

void AMQTT_Manager_Paho_Async::ConnectionLost(void* CallbackContext, char* Cause)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!Owner)
	{
		return;
	}

	const FString CauseString = UTF8_TO_TCHAR(Cause);
	Owner->Delegate_Lost.Broadcast(CauseString);
}

bool AMQTT_Manager_Paho_Async::SetSSLParams(FString In_Protocol, FPahoClientParams_Async In_Params)
{
	if (In_Params.Address.IsEmpty())
	{
		return false;
	}

	if (In_Protocol.IsEmpty())
	{
		return false;
	}

	if (In_Protocol == "wss" || In_Protocol == "mqtts" || In_Protocol == "ssl" || In_Protocol == "WSS" || In_Protocol == "MQTTS" || In_Protocol == "SSL")
	{
		this->SSL_Options = MQTTAsync_SSLOptions_initializer;
		this->SSL_Options.enableServerCertAuth = 0;
		this->SSL_Options.verify = 1;

		if (!In_Params.CAPath.IsEmpty() && FPaths::FileExists(In_Params.CAPath))
		{
			this->SSL_Options.CApath = TCHAR_TO_UTF8(*In_Params.CAPath);
		}

		else
		{
			this->SSL_Options.CApath = NULL;
		}

		if (!In_Params.Path_KeyStore.IsEmpty() && FPaths::FileExists(In_Params.Path_KeyStore))
		{
			this->SSL_Options.keyStore = TCHAR_TO_UTF8(*In_Params.Path_KeyStore);
		}

		else
		{
			this->SSL_Options.keyStore = NULL;
		}

		if (!In_Params.Path_TrustStore.IsEmpty() && FPaths::FileExists(In_Params.Path_TrustStore))
		{
			this->SSL_Options.trustStore = TCHAR_TO_UTF8(*In_Params.Path_TrustStore);
		}

		else
		{
			this->SSL_Options.trustStore = NULL;
		}

		if (!In_Params.Path_PrivateKey.IsEmpty() && FPaths::FileExists(In_Params.Path_PrivateKey))
		{

			this->SSL_Options.privateKey = TCHAR_TO_UTF8(*In_Params.Path_PrivateKey);
		}

		else
		{
			this->SSL_Options.privateKey = NULL;
		}

		this->SSL_Options.privateKeyPassword = In_Params.PrivateKeyPass.IsEmpty() ? NULL : TCHAR_TO_UTF8(*In_Params.PrivateKeyPass);
		this->SSL_Options.enabledCipherSuites = In_Params.CipherSuites.IsEmpty() ? NULL : TCHAR_TO_UTF8(*In_Params.CipherSuites);

		return true;
	}

	else
	{
		return false;
	}
}

#pragma endregion CALLBACKS

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

			if (RetVal != MQTTCLIENT_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("Description", "There was a problem while creating client. Code : " + (FString)FString::FromInt(RetVal));
				MQTTAsync_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			RetVal = MQTTAsync_setCallbacks(TempClient, this, AMQTT_Manager_Paho_Async::ConnectionLost, AMQTT_Manager_Paho_Async::MessageArrived, AMQTT_Manager_Paho_Async::DeliveryCompleted);

			if (RetVal != MQTTCLIENT_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("Description", "There was a problem while setting callbacks. Code : " + (FString)FString::FromInt(RetVal));
				MQTTAsync_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			RetVal = MQTTAsync_connect(TempClient, &this->Connection_Options);

			if (RetVal != MQTTCLIENT_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("Description", "There was a problem while making connection. Code : " + (FString)FString::FromInt(RetVal));
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

	MQTTClient_deliveryToken DeliveryToken = 0;
	const int32 QoS = FMath::Clamp((int32)In_QoS, 0, 2);
	
	MQTTAsync_message PublishedMessage = MQTTAsync_message_initializer;
	PublishedMessage.payload = TCHAR_TO_UTF8(*In_Topic);
	PublishedMessage.payloadlen = In_Payload.Len();
	PublishedMessage.qos = QoS;
	PublishedMessage.retained = In_Retained;

	MQTTAsync_responseOptions ResponseOptions = MQTTAsync_responseOptions_initializer;
	ResponseOptions.context = this;
	ResponseOptions.onSuccess = NULL;
	ResponseOptions.onSuccess5 = NULL;
	ResponseOptions.onFailure = NULL;
	ResponseOptions.onFailure5 = NULL;
	
	const int RetVal = MQTTAsync_sendMessage(this->Client, TCHAR_TO_UTF8(*In_Topic), &PublishedMessage, &ResponseOptions);
	
	const FString DescSring = RetVal == MQTTCLIENT_SUCCESS ? "Payload successfully published." : "There was a problem while publishing payload with these configurations. : " + FString::FromInt(RetVal);
	Out_Code.JsonObject->SetStringField("Description", DescSring);

	return RetVal == MQTTCLIENT_SUCCESS ? true : false;
}

bool AMQTT_Manager_Paho_Async::MQTT_Async_Subscribe(FJsonObjectWrapper& Out_Code, FString In_Topic, EMQTTQOS_Async In_QoS)
{
	return false;
}