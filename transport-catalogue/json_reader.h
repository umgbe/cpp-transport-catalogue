#pragma once

#include "request_handler.h"

/*

    Формат запросов make_base

    serialization_settings
        file : название файла, в который будет записана база
    routing_settings
        bus_wait_time : время, которое автобус ждет на каждой остановке
        bus_velocity : скорость автобуса
    render_settings
        width : ширина рисуемой карты
        height : высота рисуемой карты
        padding : расстояние от края изображения до края карты
        stop_radius : радиус кругов остановок
        line_width : толщина линий маршрутов
        stop_label_font_size : размер шрифта названий остановок
        stop_label_offset 
            [0] : смещение названия остановки по горизонтали
            [1] : смещение названия остановки по вертикали
        underlayer_color : цвет подложки названий
            Здесь и далее: цвет может задаваться в виде:    массив из 3 чисел (RGB)
                                                            массив из 4 чисел (RGBA)
                                                            текст одним словом (black, white, blue etc.)

        underlayer_width : толщина подложки названий
        color_palette 
            [0] : цвет первого маршрута
            [1] : цвет второго маршрута
            ...
            [N] : цвет последнего маршрута
        bus_label_font_size : размер шрифта названия маршрута
        bus_label_offset
            [0] : смещение названия маршрута по горизонтали
            [1] : смещение названия маршрута по вертикали
    base_requests
        [0] : 
            type : тип запроса, "Stop" или "Bus"
            name : название
            (только для Stop) latitude : широта
            (только для Stop) longitude : долгота
            (только для Stop) road_distances
                                название_остановки : дистанция до остановки, до которой можно доехать по дороге
                                название_другой_остановки : дистанция до другой остановки по дороге
                                ...
                                название_последней_остановки : дистанция до последней остановки по дороге
            (только для Bus) stops
                                [0] : название первой остановки маршрута
                                [1] : название второй остановки маршрута
                                ...
                                [N] : название последней остановки маршрута
            (только для Bus) is_roundtrip : true или false, является ли маршрут кольцевым              
                
        [1] : информация для второго запроса
        ...
        [N] : информация для последнего запроса

    
    Формат запросов process_requests

    serialization_settings
        file :
    stat_requests:
        [0] :
            id : номер запроса
            type : тип запроса, "Route", "Stop", "Bus" или "Map"
            (только для Route) from : название остановки отправления
            (только для Route) to : название остановки назначения
            (только для Stop и Bus) name : название искомой остановки или автобуса
        [1] : информация для второго запроса
        ...
        [N] : информация для последнего запроса

    Формат ответов

    [0] :
        request_id : номер запроса, на который дан ответ
        (если ответ на запрос не найден) error_message : сообщение об ошибке

        (ответ на запрос Route)
        items
            [0] :
                type : тип элемента маршрута, "Bus" или "Wait"
                time : время, затраченное на элемент маршрута
                (только для Wait) stop_name : название остановки
                (только для Bus) bus : название автобуса
                (только для Bus) span_count : количество остановок, которое нужно проехать
            [1] : информация о втором элементе маршрута
            ...
            [N] : информация о последнем элементе маршрута
        total_time : суммарное время, затраченное на маршрут

        (ответ на запрос Bus)
        curvature : отношение длины маршрута "по дорогам" к длине маршрута "по прямым линиям между остановками"
        route_length : длина маршрута "по дорогам"
        stop_count : количество остановок на маршруте
        unique_stop_count : количество уникальных остановок на маршруте

        (ответ на запрос Stop)
        buses
            [0] : название первого автобуса
            [1] : название второго автобуса
            ...
            [N] : название последнего автобуса

        (ответ на запрос Map)
        map : строка, определяющая изображение в формате SVG
    
    [1] : ответ на второй запрос
    ...
    [N] : ответ на последний запрос

*/

namespace transportCatalogue {

namespace requestsProcessing {

class JsonReader : public InterfaceIn {

public:

    JsonReader() = default;

    void ReadBaseRequests(std::istream& in);
    void ReadStatRequests(std::istream& in);

    int GetFillRequestsCount() const override;
    fillInfo GetNextFillRequest() override;

    int GetSearchRequestsCount() const override;
    searchInfo GetNextSearchRequest() override;

    mapRenderer::RenderSettings GetRenderSettings() override;

    transportRouter::RoutingSettings GetRoutingSettings() override;

    serialization::SerializationSettings GetSerializationSettings() override;

private:

    std::deque<fillInfo> fill_requests;
    std::deque<searchInfo> search_requests;
    mapRenderer::RenderSettings render_settings;
    transportRouter::RoutingSettings routing_settings;
    serialization::SerializationSettings serialization_settings; 

};

class JsonWriter : public InterfaceOut {

public:

    JsonWriter() = default;

    void AddRequestAnswer(const answerInfo& answer) override;
    void PrintAllAnswers(std::ostream& out);

private:

    std::deque<answerInfo> answers;

};

}

}