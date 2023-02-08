#ifndef Encoder_h
#define Encoder_h
#include <Encoder.h>
#include <Wire.h>
#include <vector>
#include <AS5600.h>

#define tcaAddress 0x70
AMS_5600 ams5600;

// Clase que maneja las funciones del encoder: 
class Encoder
{
private:
    float _cero = 0;
    float _valTotal = 0;
    float _valAct;
    float _valPas;
    int _actualCuadrante = 1;
    int _lastCuadrante = 1;
    int _vueltas;
    int _channel;
    const float _magnEncStepsDegrees = 4096 / 360; // 12 bits/1 vuelta.. 0.087

public:
    Encoder();
    Encoder(float valAct, float valPas, int channel);
    float getValActualDegrees(); /// Metodo para comprobar el val actual
    float getPosDeg();
    int getTurningSense(float, float);
    void selectChannel();
    int checkCuadrant(float);
    void setupCero();
    float mapVal();
    float getCero();
    float promedio(int resolucion);
    float SumDegTotal(const float);
    float mapTotal(float);
    void ActualizaPrev();
};

/// METHODS
Encoder::Encoder()
{
    _cero = 0;
    _valAct = 0;
    _valPas = 0;
    _vueltas = 0;
    _channel = 0;
}


Encoder::Encoder(float valAct, float valPas, int channel)
{
    this->_valAct = valAct;
    this->_valPas = valPas;
    this->_vueltas = 0;
    this->_cero = 0;
    this->_channel = channel;
}


void Encoder::ActualizaPrev()
{
    this->_valPas = this->_valAct;
}


float Encoder::getValActualDegrees()
{
    return this->_valAct;
}


void Encoder::selectChannel()
{
    if (this->_channel > 7)
        return;
    Wire.beginTransmission(tcaAddress);
    Wire.write(1 << this->_channel);
    Wire.endTransmission();
    delay(10);
}


float Encoder::getCero()
{
    return this->_cero;
}


float Encoder::getPosDeg()
{
    this->selectChannel();
    return (ams5600.getRawAngle() / this->_magnEncStepsDegrees);
}


int Encoder::getTurningSense(float valAct, float valPas)
{
    int sign = 0;
    if (valAct - valPas < 0)
    {
        sign = 1;
        return sign;
    }
    else
    {
        sign = 0;
        return sign;
    }
}


void Encoder::setupCero()
{
    // Toma el valor inicial que retorna el encoder, no se realiza 
    // ningun calculo mat. 

    this->_cero = Encoder::getPosDeg();
}


//El valor actual en ese momento se convierte en el 0 y desde 
//ahi comienza a contar. 
float Encoder::mapVal()
{
    float degProm = Encoder::promedio(3);
    float intervalo = (360 - this->_cero);
    if ((degProm > this->_cero) || degProm == (this->_cero))
    {
        this->_valAct = map(degProm, this->_cero, 360, 0, intervalo);
    }
    else
    {
        this->_valAct = map(degProm, 0, this->_cero, intervalo, 360);
    }
    return this->_valAct; 
}


float Encoder::SumDegTotal(const float constant)
{
    float deg = this->_valAct;

    if (0 <= deg && deg <= 90)
    {
        this->_actualCuadrante = 1;
    }
    else if (90 <= deg && deg < 180)
    {
        this->_actualCuadrante = 2;
    }
    else if (180 <= deg && deg < 270)
    {
        this->_actualCuadrante = 3;
    }
    else if (270 <= deg && deg <= 360)
    {
        this->_actualCuadrante = 4;
    }

    // Cambio de vueltas:
    if (this->_actualCuadrante != this->_lastCuadrante)
    {
        if (this->_actualCuadrante == 1 && this->_lastCuadrante == 4)
        {
            this->_vueltas++;
        }
        else if (this->_actualCuadrante == 4 && this->_lastCuadrante == 1)
        {
            this->_vueltas--;
        }
        this->_lastCuadrante = this->_actualCuadrante;
    }

    //Suma los grados cuando se da una vuelta completa: 
    this->_valTotal = this->_vueltas * 360 + this->_valAct;
    
    Encoder::ActualizaPrev();

    // Actualizacion del valor:
    float mapTotal = Encoder::mapTotal(constant);
    return mapTotal;
}


float Encoder::mapTotal(float constante)
{
    float val = (this->_valTotal * 360) / constante;
    return val;
}


float Encoder::promedio(int resolucion)
{
    float prom = 0;
    for (size_t i = 0; i < resolucion; i++)
    {
        prom = Encoder::getPosDeg() + prom;
    }
    float promedio = prom / resolucion;
    return promedio;
}
#endif
