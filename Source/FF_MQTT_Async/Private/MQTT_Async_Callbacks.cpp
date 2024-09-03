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
	Owner->Delegate_OnConnectFailure.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnDisconnect(void* CallbackContext, MQTTAsync_successData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	MQTTAsync_destroy(&Owner->Client);

	FJsonObjectWrapper Out_Result;
	Owner->Delegate_OnDisconnect.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnDisconnectFailure(void* CallbackContext, MQTTAsync_failureData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	MQTTAsync_destroy(&Owner->Client);

	FJsonObjectWrapper Out_Result;
	Owner->Delegate_OnDisconnectFailure.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnSend(void* CallbackContext, MQTTAsync_successData* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
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
	Owner->Delegate_OnSendFailure.Broadcast(Out_Result);
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
	Owner->Delegate_OnConnectFailure.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnDisconnect5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	MQTTAsync_destroy(&Owner->Client);

	FJsonObjectWrapper Out_Result;
	Owner->Delegate_OnDisconnect.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnDisconnectFailure5(void* CallbackContext, MQTTAsync_failureData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	MQTTAsync_destroy(&Owner->Client);

	FJsonObjectWrapper Out_Result;
	Owner->Delegate_OnDisconnectFailure.Broadcast(Out_Result);
}

void AMQTT_Manager_Paho_Async::OnSend5(void* CallbackContext, MQTTAsync_successData5* Response)
{
	AMQTT_Manager_Paho_Async* Owner = Cast<AMQTT_Manager_Paho_Async>((AMQTT_Manager_Paho_Async*)CallbackContext);

	if (!IsValid(Owner))
	{
		return;
	}

	FJsonObjectWrapper Out_Result;
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
	Owner->Delegate_OnSendFailure.Broadcast(Out_Result);
}
#pragma endregion V5_CALLBACKS