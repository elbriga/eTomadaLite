#pragma once
#include <Arduino.h>

// ============================================================
// ESP8266
// ============================================================
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <Updater.h>

using ETomadaWebServer = ESP8266WebServer;

// ============================================================
// LibreTiny - setar buildFlags para cada env -D ETOMADA_LIBRETINY
// ============================================================
#elif defined(ETOMADA_LIBRETINY)
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <mDNS.h>
#include <Update.h>
using ETomadaWebServer = WebServer;

// ============================================================
// Plataforma desconhecida
// ============================================================
#else
#error "Plataforma nao suportada pelo eTomada Lite"
#endif

inline void platformMDNSUpdate()
{
#if defined(ESP8266)
    MDNS.update();
#endif
}
