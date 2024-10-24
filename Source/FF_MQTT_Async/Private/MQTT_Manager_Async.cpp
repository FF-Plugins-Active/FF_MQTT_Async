// Fill out your copyright notice in the Description page of Project Settings.

#include "MQTT_Manager_Async.h"

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
	this->MQTT_Async_Destroy();
	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void AMQTT_Manager_Paho_Async::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FPahoClientParams AMQTT_Manager_Paho_Async::GetClientParams()
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
		MQTTAsync_disconnectOptions Disconnect_Options = MQTTAsync_disconnectOptions_initializer;
		Disconnect_Options.context = this;
		Disconnect_Options.timeout = 10000;

		if (this->Client_Params.Version == EMQTTVERSION::V_5)
		{
			Disconnect_Options.onSuccess5 = OnDisconnect5;
			Disconnect_Options.onFailure5 = OnDisconnectFailure5;
			Disconnect_Options.reasonCode = MQTTREASONCODE_DISCONNECT_WITH_WILL_MESSAGE;
		}

		else
		{
			Disconnect_Options.onSuccess = OnDisconnect;
			Disconnect_Options.onFailure = OnDisconnectFailure;
		}

		try
		{
			MQTTAsync_disconnect(this->Client, &Disconnect_Options);
		}

		catch (const std::exception& Exception)
		{
			const FString ExceptionString = Exception.what();
			UE_LOG(LogTemp, Warning, TEXT("Disconnect exception : %s"), *ExceptionString);
		}
	}

	MQTTAsync_destroy(&this->Client);
}

bool AMQTT_Manager_Paho_Async::MQTT_Async_Init(FJsonObjectWrapper& Out_Code, FPahoClientParams In_Params)
{
	Out_Code.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho_Async");
	Out_Code.JsonObject->SetStringField("FunctionName", "MQTT_Async_Init");
	Out_Code.JsonObject->SetStringField("AdditionalInfo", "");
	Out_Code.JsonObject->SetStringField("ErrorCode", "");

	if (this->Client)
	{
		Out_Code.JsonObject->SetStringField("Description", "Client already initialized.");
		return false;
	}

	if (MQTTAsync_isConnected(this->Client))
	{
		Out_Code.JsonObject->SetStringField("Description", "Client already connected.");
		return false;
	}

	FString ParameterReason;
	if (!In_Params.IsParamsValid(ParameterReason))
	{
		Out_Code.JsonObject->SetStringField("Description", ParameterReason);
		return false;
	}

	MQTTAsync Temp_Client = nullptr;
	int RetVal = -1;

	FString Protocol = In_Params.GetProtocol();

	if (In_Params.Version == EMQTTVERSION::V_5)
	{
		MQTTAsync_createOptions Create_Options = MQTTAsync_createOptions_initializer;
		Create_Options.MQTTVersion = MQTTVERSION_5;

		RetVal = MQTTAsync_createWithOptions(&Temp_Client, TCHAR_TO_UTF8(*In_Params.Address), TCHAR_TO_UTF8(*In_Params.ClientId), MQTTCLIENT_PERSISTENCE_NONE, NULL, &Create_Options);

		if (Protocol == "wss" || Protocol == "ws")
		{
			this->Connection_Options = MQTTAsync_connectOptions_initializer5_ws;
		}

		else
		{
			this->Connection_Options = MQTTAsync_connectOptions_initializer5;
		}

		this->Connection_Options.cleanstart = 1;
		this->Connection_Options.onSuccess5 = OnConnect5;
		this->Connection_Options.onFailure5 = OnConnectFailure5;
	}

	else
	{
		RetVal = MQTTAsync_create(&Temp_Client, TCHAR_TO_UTF8(*In_Params.Address), TCHAR_TO_UTF8(*In_Params.ClientId), MQTTCLIENT_PERSISTENCE_NONE, NULL);

		if (Protocol == "wss" || Protocol == "ws")
		{
			this->Connection_Options = MQTTAsync_connectOptions_initializer_ws;
		}

		else
		{
			this->Connection_Options = MQTTAsync_connectOptions_initializer;
		}

		this->Connection_Options.cleansession = 1;
		this->Connection_Options.onSuccess = OnConnect;
		this->Connection_Options.onFailure = OnConnectFailure;
	}

	this->Connection_Options.context = this;
	this->Connection_Options.keepAliveInterval = In_Params.KeepAliveInterval;
	this->Connection_Options.username = TCHAR_TO_UTF8(*In_Params.UserName);
	this->Connection_Options.password = TCHAR_TO_UTF8(*In_Params.Password);
	this->Connection_Options.MQTTVersion = (int32)In_Params.Version;
	
	if (this->SetSSLParams(Protocol, In_Params.SSL_Options))
	{
		this->Connection_Options.ssl = &this->SSL_Options;
		Out_Code.JsonObject->SetStringField("AdditionalInfo", "SSL parameters set.");
	}

	else
	{
		Out_Code.JsonObject->SetStringField("AdditionalInfo", "SSL parameters couldn't set.");
	}	

	if (RetVal != MQTTASYNC_SUCCESS)
	{
		Out_Code.JsonObject->SetStringField("Description", "There was a problem while creating client.");
		Out_Code.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));

		MQTTAsync_destroy(&Temp_Client);
		return false;
	}

	RetVal = MQTTAsync_setCallbacks(Temp_Client, this, AMQTT_Manager_Paho_Async::ConnectionLost, AMQTT_Manager_Paho_Async::MessageArrived, AMQTT_Manager_Paho_Async::DeliveryCompleted);

	if (RetVal != MQTTASYNC_SUCCESS)
	{
		Out_Code.JsonObject->SetStringField("Description", "There was a problem while setting callbacks.");
		Out_Code.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));

		MQTTAsync_destroy(&Temp_Client);
		return false;
	}

	RetVal = MQTTAsync_connect(Temp_Client, &this->Connection_Options);

	if (RetVal != MQTTASYNC_SUCCESS)
	{
		Out_Code.JsonObject->SetStringField("Description", "There was a problem while making connection.");
		Out_Code.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));

		MQTTAsync_destroy(&Temp_Client);
		return false;
	}

	Out_Code.JsonObject->SetStringField("Description", "Connection successful.");
	Out_Code.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));

	this->Client = Temp_Client;
	this->Client_Params = In_Params;

	return true;
}

bool AMQTT_Manager_Paho_Async::MQTT_Async_Publish(FJsonObjectWrapper& Out_Code, FString In_Topic, FString In_Payload, EMQTTQOS In_QoS, int32 In_Retained)
{
	Out_Code.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho_Async");
	Out_Code.JsonObject->SetStringField("FunctionName", "MQTT_Async_Publish");
	Out_Code.JsonObject->SetStringField("AdditionalInfo", "");
	Out_Code.JsonObject->SetStringField("ErrorCode", "");

	if (!this->Client)
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not valid.");
		return false;
	}

	if (!MQTTAsync_isConnected(this->Client))
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not connected.");
		Out_Code.JsonObject->SetStringField("AdditionalInfo", "Try to give some delay before using this or use it after \"Delegate OnConnect\"");
		return false;
	}

	MQTTAsync_message PublishedMessage = MQTTAsync_message_initializer;
	PublishedMessage.payload = (void*)TCHAR_TO_UTF8(*In_Payload);
	PublishedMessage.payloadlen = In_Payload.Len();
	PublishedMessage.qos = 0;
	PublishedMessage.retained = 0;

	MQTTAsync_responseOptions ResponseOptions = MQTTAsync_responseOptions_initializer;
	ResponseOptions.context = this;
	
	if (this->Client_Params.Version == EMQTTVERSION::V_5)
	{
		ResponseOptions.onSuccess5 = OnSend5;
		ResponseOptions.onFailure5 = OnSendFailure5;
	}

	else
	{
		ResponseOptions.onSuccess = OnSend;
		ResponseOptions.onFailure = OnSendFailure;
	}
	
	const int RetVal = MQTTAsync_sendMessage(this->Client, TCHAR_TO_UTF8(*In_Topic), &PublishedMessage, &ResponseOptions);

	const FString Description = RetVal == MQTTASYNC_SUCCESS ? "Message successfully published." : "There was a problem while publishing message.";
	Out_Code.JsonObject->SetStringField("Description", Description);
	Out_Code.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));
	
	return RetVal == MQTTASYNC_SUCCESS ? true : false;
}

bool AMQTT_Manager_Paho_Async::MQTT_Async_Subscribe(FJsonObjectWrapper& Out_Code, FString In_Topic, EMQTTQOS In_QoS)
{
	Out_Code.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho_Async");
	Out_Code.JsonObject->SetStringField("FunctionName", "MQTT_Async_Subscribe");
	Out_Code.JsonObject->SetStringField("AdditionalInfo", "");
	Out_Code.JsonObject->SetStringField("ErrorCode", "");

	if (!this->Client)
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not valid.");
		return false;
	}

	if (!MQTTAsync_isConnected(this->Client))
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not connected.");
		Out_Code.JsonObject->SetStringField("AdditionalInfo", "Try to give some delay before using this or use it after \"Delegate OnConnect\"");
		return false;
	}

	MQTTAsync_responseOptions Response_Options = MQTTAsync_responseOptions_initializer;
	Response_Options.context = this;

	if (this->Client_Params.Version == EMQTTVERSION::V_5)
	{
		Response_Options.onSuccess5 = NULL;
		Response_Options.onFailure5 = NULL;
	}

	else
	{
		Response_Options.onSuccess = NULL;
		Response_Options.onFailure = NULL;
	}

	const int RetVal = MQTTAsync_subscribe(this->Client, TCHAR_TO_UTF8(*In_Topic), (int32)In_QoS, & Response_Options);

	const FString Description = RetVal == MQTTASYNC_SUCCESS ? "Target successfully subscribed." : "There was a problem while subscribing target.";
	Out_Code.JsonObject->SetStringField("Description", Description);
	Out_Code.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));

	return RetVal == MQTTASYNC_SUCCESS ? true : false;
}

bool AMQTT_Manager_Paho_Async::MQTT_Async_Unsubscribe(FJsonObjectWrapper& Out_Code, FString In_Topic)
{
	Out_Code.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho_Async");
	Out_Code.JsonObject->SetStringField("FunctionName", "MQTT_Async_Unsubscribe");
	Out_Code.JsonObject->SetStringField("AdditionalInfo", "");
	Out_Code.JsonObject->SetStringField("ErrorCode", "");

	if (!this->Client)
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not valid.");
		return false;
	}

	if (!MQTTAsync_isConnected(this->Client))
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not connected.");
		Out_Code.JsonObject->SetStringField("AdditionalInfo", "Try to give some delay before using this or use it after \"Delegate OnConnect\"");
		return false;
	}

	MQTTAsync_responseOptions Response_Options = MQTTAsync_responseOptions_initializer;
	Response_Options.context = this;

	if (this->Client_Params.Version == EMQTTVERSION::V_5)
	{
		Response_Options.onSuccess5 = OnUnSubscribe5;
		Response_Options.onFailure5 = OnUnSubscribeFailure5;
	}

	else
	{
		Response_Options.onSuccess = OnUnSubscribe;
		Response_Options.onFailure = OnUnSubscribeFailure;
	}

	const int RetVal = MQTTAsync_unsubscribe(this->Client, TCHAR_TO_UTF8(*In_Topic), &Response_Options);

	const FString Description = RetVal == MQTTASYNC_SUCCESS ? "Target successfully unsubscribed." : "There was a problem while unsubscribing target.";
	Out_Code.JsonObject->SetStringField("Description", Description);
	Out_Code.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));

	return RetVal == MQTTASYNC_SUCCESS ? true : false;
}
