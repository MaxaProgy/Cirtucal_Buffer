# Лабораторная работа 8

STL - совместимый контейнер "Циклический буфер", с поддержкой возможности расширения и фиксированным размером.

[Циклический буфер](https://en.wikipedia.org/wiki/Circular_buffer) - структура данных, которая часто применяется для буферизации в задачах по передачи и обработки потока данных.

## Задача

Реализовать два класса:
CCirtucalBuffer и CCircularBufferExt - для циклического буфера и циклического буфера с возможностью расширения (см ниже).

Реализовать циклический буфер для хранения данных произвольного типа в виде stl-совместимого контейнера.
Шаблон класс(ы) должен параметризироваться типом хранимого значения и  аллокатором.

## Требования

Контейнер должен удовлетворять [следующим требованиям](https://en.cppreference.com/w/cpp/named_req/Container) для stl-контейнера.
А также [требования для последовательного контейнера](https://en.cppreference.com/w/cpp/named_req/SequenceContainer)

Исключая rvalue и move-семантику.

## Итератор

Класс должен предоставлять итератор произвольного доступа.

С требования для подобного итератора можно ознакомиться [здесь](https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator).

## Кольцевой буфер с расширением максимального размера.

В учебных целях, класс CCircularBufferExt должен обладать функциональностью для расширения свой максимального размера.
Должно быть реализовано след поведение: в случае достижения размера кольцевого буфера максимального возможного своего размера, значение максимального размера должно удваиваться.

## Тесты

Вашу реализацию Кольцевого Буфера требуется покрыть тестами, с помощью фреймворка Google Test.
Документация и примеры можно найти вот [тут](http://google.github.io/googletest).

Способ подключения и запуска тестов можно посмотреть в предыдущих лабораторных.

Тесты также являются частью задания, поэтому покрытие будет влиять на максимальный балл.


## Ограничения

* Запрещено использовать стандартные контейнеры










