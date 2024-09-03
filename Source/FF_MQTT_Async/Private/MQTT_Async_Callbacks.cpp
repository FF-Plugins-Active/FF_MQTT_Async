#include "MQTT_Manager_Paho_Async.h"

bool AMQTT_Manager_Paho_Async::SetSSLParams(FString In_Protocol, FPahoClientParams_Async In_Params)
{
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

#pragma region MAINCALLBACKS
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

#pragma endregion MAINCALLBACKS

#pragma region V3_CALLBACKS
void AMQTT_Manager_Paho_Async::OnConnect(void* CallbackContext, MQTTAsync_successData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("MQTT_Version", FString::FromInt(Response->alt.connect.MQTTVersion));
	Out_Result.JsonObject->SetStringField("Session_Present", FString::FromInt(Response->alt.connect.sessionPresent));
	Out_Result.JsonObject->SetStringField("Server_Uri", UTF8_TO_TCHAR(Response->alt.connect.serverURI));
	
	Owner->Delegate_OnConnect.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnConnectFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("Code", FString::FromInt(Response->code));
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	Owner->Delegate_OnConnectFailure.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnDisconnect(void* CallbackContext, MQTTAsync_successData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("MQTT_Version", FString::FromInt(Response->alt.connect.MQTTVersion));
	Out_Result.JsonObject->SetStringField("Session_Present", FString::FromInt(Response->alt.connect.sessionPresent));
	Out_Result.JsonObject->SetStringField("Server_Uri", UTF8_TO_TCHAR(Response->alt.connect.serverURI));

	Owner->Delegate_OnDisconnect.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnDisconnectFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("Code", FString::FromInt(Response->code));
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	Owner->Delegate_OnDisconnectFailure.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnSend(void* CallbackContext, MQTTAsync_successData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	const int PayloadLenght = Response->alt.pub.message.payloadlen;
	void* Payload = Response->alt.pub.message.payload;

	int32 Index = 0;
	int32 Length = 0x7FFFFFFF;

	if (Index < 0)
	{
		Length += Index;
		Index = 0;
	}

	if (Length > PayloadLenght - Index)
	{
		Length = PayloadLenght - Index;
	}

	const FUTF8ToTCHAR Src(reinterpret_cast<const ANSICHAR*>((uint8*)Payload + Index), Length);
	const FString UTF8 = FString(Src.Length(), Src.Get());

	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("Topic", Response->alt.pub.destinationName);
	Out_Result.JsonObject->SetStringField("Dup_Flag", FString::FromInt(Response->alt.pub.message.dup));
	Out_Result.JsonObject->SetStringField("Message_Id", FString::FromInt(Response->alt.pub.message.msgid));
	Out_Result.JsonObject->SetStringField("Payload", UTF8);
	Out_Result.JsonObject->SetStringField("Payload_Lenght", FString::FromInt(PayloadLenght));
	Out_Result.JsonObject->SetStringField("Payload_Lenght", FString::FromInt(Response->alt.pub.message.qos));
	Out_Result.JsonObject->SetStringField("Payload_Lenght", FString::FromInt(Response->alt.pub.message.retained));

	Owner->Delegate_OnSend.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnSendFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("Code", FString::FromInt(Response->code));
	Out_Result.JsonObject->SetStringField("Message", Response->message);

	Owner->Delegate_OnSendFailure.Broadcast(Out_Result);
}
void AMQTT_Manager_Paho_Async::OnUnSubscribe(void* CallbackContext, MQTTAsync_successData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
}

void AMQTT_Manager_Paho_Async::OnUnSubscribeFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
}
#pragma endregion V3_CALLBACKS

#pragma region V5_CALLBACKS
void AMQTT_Manager_Paho_Async::OnConnect5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
	
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("MQTT_Version", FString::FromInt(Response->alt.connect.MQTTVersion));
	Out_Result.JsonObject->SetStringField("Session_Present", FString::FromInt(Response->alt.connect.sessionPresent));
	Out_Result.JsonObject->SetStringField("Server_Uri", UTF8_TO_TCHAR(Response->alt.connect.serverURI));
	Out_Result.JsonObject->SetStringField("Reason_Code", MQTTReasonCode_toString(Response->reasonCode));

	Owner->Delegate_OnConnect.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnConnectFailure5(void* CallbackContext, MQTTAsync_failureData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}
	
	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("Code", FString::FromInt(Response->code));
	Out_Result.JsonObject->SetStringField("Message", Response->message);
	Out_Result.JsonObject->SetStringField("Reason_Code", MQTTReasonCode_toString(Response->reasonCode));
	Out_Result.JsonObject->SetStringField("Packet_Type", FString::FromInt(Response->packet_type));

	Owner->Delegate_OnConnectFailure.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnDisconnect5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("MQTT_Version", FString::FromInt(Response->alt.connect.MQTTVersion));
	Out_Result.JsonObject->SetStringField("Session_Present", FString::FromInt(Response->alt.connect.sessionPresent));
	Out_Result.JsonObject->SetStringField("Server_Uri", UTF8_TO_TCHAR(Response->alt.connect.serverURI));
	Out_Result.JsonObject->SetStringField("Reason_Code", MQTTReasonCode_toString(Response->reasonCode));

	Owner->Delegate_OnDisconnect.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnDisconnectFailure5(void* CallbackContext, MQTTAsync_failureData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("Code", FString::FromInt(Response->code));
	Out_Result.JsonObject->SetStringField("Message", Response->message);
	Out_Result.JsonObject->SetStringField("Reason_Code", MQTTReasonCode_toString(Response->reasonCode));
	Out_Result.JsonObject->SetStringField("Packet_Type", FString::FromInt(Response->packet_type));

	Owner->Delegate_OnDisconnectFailure.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnSend5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	const int PayloadLenght = Response->alt.pub.message.payloadlen;
	void* Payload = Response->alt.pub.message.payload;

	int32 Index = 0;
	int32 Length = 0x7FFFFFFF;

	if (Index < 0)
	{
		Length += Index;
		Index = 0;
	}

	if (Length > PayloadLenght - Index)
	{
		Length = PayloadLenght - Index;
	}

	const FUTF8ToTCHAR Src(reinterpret_cast<const ANSICHAR*>((uint8*)Payload + Index), Length);
	const FString UTF8 = FString(Src.Length(), Src.Get());

	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("Topic", Response->alt.pub.destinationName);
	Out_Result.JsonObject->SetStringField("Dup_Flag", FString::FromInt(Response->alt.pub.message.dup));
	Out_Result.JsonObject->SetStringField("Message_Id", FString::FromInt(Response->alt.pub.message.msgid));
	Out_Result.JsonObject->SetStringField("Payload", UTF8);
	Out_Result.JsonObject->SetStringField("Payload_Lenght", FString::FromInt(PayloadLenght));
	Out_Result.JsonObject->SetStringField("Payload_Lenght", FString::FromInt(Response->alt.pub.message.qos));
	Out_Result.JsonObject->SetStringField("Payload_Lenght", FString::FromInt(Response->alt.pub.message.retained));

	Owner->Delegate_OnSend.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnSendFailure5(void* CallbackContext, MQTTAsync_failureData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
	Out_Result.JsonObject->SetStringField("Token", FString::FromInt(Response->token));
	Out_Result.JsonObject->SetStringField("Code", FString::FromInt(Response->code));
	Out_Result.JsonObject->SetStringField("Message", Response->message);
	Out_Result.JsonObject->SetStringField("Reason_Code", MQTTReasonCode_toString(Response->reasonCode));
	Out_Result.JsonObject->SetStringField("Packet_Type", FString::FromInt(Response->packet_type));

	Owner->Delegate_OnSendFailure.Broadcast(Out_Result);
}
void AMQTT_Manager_Paho_Async::OnUnSubscribe5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
}
void AMQTT_Manager_Paho_Async::OnUnSubscribeFailure5(void* CallbackContext, MQTTAsync_failureData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
}
#pragma endregion V5_CALLBACKS