// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "MQTT_Includes.h"
#include "MQTT_Includes_Paho.h"

#include "MQTT_Manager_Async.generated.h"

UCLASS()
class FF_MQTT_ASYNC_API AMQTT_Manager_Paho_Async : public AActor
{
	GENERATED_BODY()
	
private:	

	MQTTAsync Client = nullptr;
	MQTTAsync_connectOptions Connection_Options;
	MQTTAsync_SSLOptions SSL_Options;
	FPahoClientParams Client_Params;

#pragma region CALLBACKS

	virtual bool SetSSLParams(FString In_Protocol, FPahoClientParams In_Params);

	static void OnConnect(void* CallbackContext, MQTTAsync_successData* Response);
	static void OnConnectFailure(void* CallbackContext, MQTTAsync_failureData* Response);
	static void OnDisconnect(void* CallbackContext, MQTTAsync_successData* Response);
	static void OnDisconnectFailure(void* CallbackContext, MQTTAsync_failureData* Response);
	static void OnSend(void* CallbackContext, MQTTAsync_successData* Response);
	static void OnSendFailure(void* CallbackContext, MQTTAsync_failureData* Response);
	static void OnUnSubscribe(void* CallbackContext, MQTTAsync_successData* Response);
	static void OnUnSubscribeFailure(void* CallbackContext, MQTTAsync_failureData* Response);

	static void OnConnect5(void* CallbackContext, MQTTAsync_successData5* Response);
	static void OnConnectFailure5(void* CallbackContext, MQTTAsync_failureData5* Response);
	static void OnDisconnect5(void* CallbackContext, MQTTAsync_successData5* Response);
	static void OnDisconnectFailure5(void* CallbackContext, MQTTAsync_failureData5* Response);
	static void OnSend5(void* CallbackContext, MQTTAsync_successData5* Response);
	static void OnSendFailure5(void* CallbackContext, MQTTAsync_failureData5* Response);
	static void OnUnSubscribe5(void* CallbackContext, MQTTAsync_successData5* Response);
	static void OnUnSubscribeFailure5(void* CallbackContext, MQTTAsync_failureData5* Response);

	static void DeliveryCompleted(void* CallbackContext, MQTTAsync_token DeliveredToken);
	static int MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTAsync_message* Message);
	static void ConnectionLost(void* CallbackContext, char* Cause);
	
#pragma endregion CALLBACKS

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	// Sets default values for this actor's properties.
	AMQTT_Manager_Paho_Async();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Delivered Delegate_Delivered;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Arrived Delegate_Arrived;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Lost Delegate_Lost;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_OnConnect Delegate_OnConnect;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_OnConnectFailure Delegate_OnConnectFailure;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_OnDisconnect Delegate_OnDisconnect;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_OnDisconnectFailure Delegate_OnDisconnectFailure;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_OnSend Delegate_OnSend;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_OnSendFailure Delegate_OnSendFailure;

	UFUNCTION(BlueprintPure, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Get Client Parameters"))
	virtual FPahoClientParams GetClientParams();

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Destroy", ToolTip = "", KeyWords = "mqtt, async, paho, client, destroy, close, disconnect"))
	virtual void MQTT_Async_Destroy();

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Init", ToolTip = "Don't attach publishers or subscribers immediately after this. Use some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, async, paho, client, init, initialize, start, connect"))
	virtual bool MQTT_Async_Init(FJsonObjectWrapper& Out_Code, FPahoClientParams In_Params);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Publish", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, async, paho, client, publish, publisher"))
	virtual bool MQTT_Async_Publish(FJsonObjectWrapper& Out_Code, FString In_Topic, FString In_Payload, EMQTTQOS In_QoS = EMQTTQOS::QoS_0, int32 In_Retained = 0);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Subscribe", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, async, paho, client, subscribe, subscriber"))
	virtual bool MQTT_Async_Subscribe(FJsonObjectWrapper& Out_Code, FString In_Topic, EMQTTQOS In_QoS = EMQTTQOS::QoS_0);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Async - Unsubscribe", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, async, paho, client, unsubscribe, subscriber"))
	virtual bool MQTT_Async_Unsubscribe(FJsonObjectWrapper& Out_Code, FString In_Topic);

};
