# er9x-frsky-mavlink
Automatically exported from code.google.com/p/er9x-frsky-mavlink
About
This project contain two firmware: Transmiter firmware (based on er9x) and firmware for Arduino Pro Mini (based on APM-Mavlink-to-FrSky for Taranis) for transmitting telemetry data from APM via Mavlink protocol to Turnigy 9X(R) display over FrSky modules supported original FrSky? telemetry protocol without 3DR Radio or XBee.

Ardupilot -> Arduino Pro Mini -> FrSky? receiver -> er9x-based transmitter  

Required hardware
Arduino Pro Mini 5V version recommended, but 3.3V version working well.

USB->TTL 5V convertor based on FTDI, PL2303 or any other chip. Required DTR pin for flashing Arduino. (With USB cable)

USBasp AVR programmer for flashing Turnigy 9XR

10 pin to 6 pin adapter for USBasp

Telemetry cable for APM

Tested configuration
Turnigy 9XR Transmitter Mode 2 (or any other based on Turnigy 9X) with FrSky? hardware MOD

You can using any tutorial for making this mod 1 2 3 4 5

FrSky DJT and D8R-IIplus modules (or any other with FrSky? telemetry support)

Ardupilot compatible Flight Controller v2.52 (or any other controller with APM firmware)

Required software
eePe for flashing you radio

XLoader (or any other Arduino HEX uploader) OR Arduino IDE for flashing Arduino from source code.

For using Arduino IDE you must import libraries

[https://code.google.com/p/er9x-frsky-mavlink/ English] [https://code.google.com/p/er9x-frsky-mavlink/wiki/Russian Russian]

==Обсуждение этого проекта на форумах:==
[http://forum.rcdesign.ru/f4/thread364947.html rcdesign.ru] на русском

[http://www.rcgroups.com/forums/showthread.php?t=2170262 rcgroups.com] на английском

[http://openrcforums.com/forum/viewtopic.php?f=5&t=5485&p=87943 openrcforums.com] на английском


Этот проект позволяет пилотам отображать специфичные для Ardupilot (APM) данные на экране аппы Turnigy 9X и ее клонов без использования дополнительного радиоканала телеметрии на базе 3DR Radio или XBee. Передача данных происходит по радиоканалу телеметрии, реализованной в приемниках и ВЧ модулях FrSky или openLRSng. Для владельцев 3DR Radio или XBee появляется возможность отображения телеметрии как на наземной станции, так и на экране аппы.
Телеметрия на страницах 1-4 отображает стандартные данные FrSky, включая данные хаба FrSky (за исключением temp1 и temp2)
Успешно работает совместно с OSD и 3DR Radio.

===8-ми канальное управление подсветкой (работает и без FrSky оборудования, не требуется прошивка аппы)===

Подробное описание настройки подсветки ниже, в вопросах и ответах

===Вам подойдет это решение, если Вы счастливый владелец следующего оборудования:===

1. Полетный контроллер c прошивкой [http://copter.ardupilot.com APM:Copter] или клон

2. Приемник FrSky с поддержкой телеметрии [http://www.frsky-rc.com/product/category.php?cate_id=16 D серии] ([http://www.frsky-rc.com/product/category.php?cate_id=17 S серия] с протоколом SPort не поддерживается) или Приемник openLRSng

3. ВЧ модуль передатчика FrSky [http://www.frsky-rc.com/product/category.php?cate_id=14 D серии] или модуль openLRSng 

4. Аппаратура [http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=31544&aff=233755 Turnigy 9XR] или любой другой клон 9X


===APM -> Arduino Pro Mini -> FrSky приемник -> Turnigy 9X с ВЧ модулем FrSky===

https://er9x-frsky-mavlink.googlecode.com/svn/trunk/mavlink-frsky.jpg

https://er9x-frsky-mavlink.googlecode.com/svn/trunk/displays.jpg


===Необходимое дополнительное оборудование===

[http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=26869&aff=233755 Arduino Pro Mini] рекомендуем версию с питанием 5В, хотя версия 3.3В тоже работает. Требуется контроллер не ниже 328 (в старые 168 прошивка не влезает). Arduino Nano работает, но требуется доработка, читайте форум.

[http://www.ebay.com/sch/i.html?_odkw=usb+ttl&_osacat=0&_from=R40&_trksid=p2045573.m570.l1313.TR12.TRC2.A0.H0.Xusb+ttl+5v&_nkw=usb+ttl+5v&_sacat=0 USB->TTL 5V convertor] на чипах FTDI, PL2303 или других. Для программирования Arduinio требуется распаянный пин DTE (DTR). Не забудьте про USB кабель.

[http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=27990&aff=233755 USBasp AVR programmer] для прошивки аппы Turnigy 9X. Можете использовать любой другой программатор, которым Вы привыкли пользоваться.

[http://www.ebay.com/sch/i.html?_odkw=usbasp+programmer&_osacat=0&_from=R40&_trksid=p2045573.m570.l1313.TR0.TRC0.H0.Xusbasp+adapter+6pin&_nkw=usbasp+adapter+6pin&_sacat=0 10 pin to 6 pin adapter for USBasp] Удобно использовать такой переходник для быстрого подсоединения программатора к аппе.

[http://www.ebay.com/sch/i.html?_odkw=telemetry+cable+for+apm&_osacat=0&_from=R40&_trksid=p2045573.m570.l1313.TR0.TRC0.H0.Xtelemetry+cable+apm&_nkw=telemetry+cable+apm&_sacat=0 Telemetry cable for APM] Для тех, кто любит чтобы все было аккуратно и на разъемах, может понравится подобный кабель. Менее дотошные могут припаять 4 провода к разъему телеметрии APM с обратной стороны.


Проект содержит две прошивки: одну для Arduino Pro Mini и прошивки для аппы Turnigy 9X для разных контроллеров (MEGA64, MEGA128 и MEGA2561)

Прошивка для Turnigy 9X создана на основе популярной [https://code.google.com/p/er9x/ er9x прошивки r812] (добавлен экран №5 отображения данных, принимаемых от Ardupilot, остальное без изменений)

Прошивка для Arduino основана на проекте [https://github.com/vizual54/APM-Mavlink-to-FrSky APM-Mavlink-to-FrSky for Taranis]


=Отображаемые параметры=

===Верхняя строка:===

Остаток заряда бортовой батареи (по данным APM)

Напряжение с [http://store.3drobotics.com/products/apm-power-module-with-xt60-connectors модуля питания]

Напряжение процессора полетного контроллера

Напряжение батареи аппы

===Левая колонка:===

Arming/Disarming

GPS: NO Fix, 2D Fix, 3D Fix, DGPS и RTK

sat  - Количество видимых спутников

hdop - [http://ru.wikipedia.org/wiki/DOP HDOP] в метрах

Индикатор приема данных (heartbeat)

Отображение "здоровья" (health) сенсоров (если включены): 3D gyro, 3D accelerometer, 3D magnetometer, absolute pressure, differential pressure - air speed, GPS, optical flow, geofence, AHRS subsystem health


===Правая колонка:===
Полетный режим (поддерживаются все полетные режимы текущей версии 1.3.3-dev ардукоптера, включая FLIP, AutoTune и PosHold)

alt  - Высота в метрах по барометру

gAl  - Высота в метрах по GPS

dth  - Дистанция до точки взлета (home position)

wp   - Отображение расстояние до следуещей точки маршрута и ее номер

thr% - Отображение газа (данные не по положению стика газа, а величина, передаваемая непосредственно на двигатели самим автопилотом)


Отображение 24 сообщений системы уровня SEVERITY_HIGH, включая [http://copter.ardupilot.com/wiki/prearm_safety_check/ PreArm safety check], AutoTune, а также новые ESC Cal, Low Battery и Lost GPS

В центре экрана вы найдете квадрат, за которым прижилось название "Радар" на котором вы увидите:

Самая длинная линия      - положение носа аппарата. В режиме Disarmed показывает положение носа по компасу (север - вверху). В режиме Arming показывает положение относительно взлета (как в SIMPLE режимах)

Короткая одинарная линия - направление на текущую точку маршрута

Короткая тройная линия   - направление на точку взлета (home position)

Направление "домой" рассчитывается как и расстояние "до дома" по разнице текущих GPS координат и точки взлета (алгоритм тот же, что и в различных OSD).

===Нижняя строка:===
RX и TX - уровень сигнала

cpu - загрузка процессора полетного контроллера

Ток в амперах, измеренный [http://store.3drobotics.com/products/apm-power-module-with-xt60-connectors модулем питания]


===Конфигурация оборудования, на которой производилось тестирование===

 * [http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=31544&aff=233755 Turnigy 9XR Transmitter Mode 2] (подойдет любая другая аппаратура клона Turnigy 9X)

 * [http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=14355&aff=233755 FrSky DJT and D8R-IIplus modules] Комплект приемник и модуль для аппы (можете использовать другие модули FrSky с поддеркой телеметрии)

 * [http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=37328&aff=233755 Ardupilot v2.52 compatible Flight Controller] (или любой другой клон, поддерживающий прошивки [http://ardupilot.com/ APM])

===Необходимое программное обеспечение===
[https://code.google.com/p/eepe/ eePe] для прошивки аппы и сохранения/редактирования настроек.

[http://russemotto.com/xloader/ XLoader] (или другая программа, которая позволяет заливать HEX файлы в Arduino) ИЛИ

[http://arduino.cc/en/Main/Software Arduino IDE 1.0.6] для прошивки Arduino из исходных кодов. При использовании Arduino IDE не забудьте сделать импорт [https://code.google.com/p/er9x-frsky-mavlink/source/browse/#svn/trunk/source/mavlink-driver/libraries библиотек]

===Порядок действий===

1. У вас должен быть сделан так называемый "FrSky Hardware Mod" для Turnigy 9X. Наиболее оптимальный, на мой взгляд, способ описан [http://forum.rcdesign.ru/blogs/105113/blog16284.html здесь]. Если этот способ вас не устраивает, то привожу ссылки на другие разнообразные инструкции (на английском языке) [http://er9x.googlecode.com/svn/trunk/doc/FrSky%20Telemetry%20%20details.pdf 1] [http://code.google.com/p/gruvin9x/wiki/FrskyInterfacing 2] [http://er9x.googlecode.com/svn/trunk/doc/TelemetryMods.pdf 3] [http://www.flickr.com/photos/erezraviv/5830896454/in/photostream 4] [http://er9x.googlecode.com/svn/trunk/doc/FRSKYTelemetry.pdf 5]

2. Определяем процессор вашей аппаратуры 64, 128 или 2561 (вскрываем аппу, смотрим через лупу название самой большой микросхемы)

3. Настраиваем [https://code.google.com/p/eepe/ eePe] правильно указав модель процессора

4. Сохраняем все настройки аппы в файл с помощью [http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=27990&aff=233755 программатора] и [https://code.google.com/p/eepe/ eePe] (Read Memory From TX, Файл - Сохранить как...)

5. Прошиваем аппу прошивкой, соответствующей модели процессора. Для mega64 берем [https://er9x-frsky-mavlink.googlecode.com/svn/trunk/bin/er9x-64.hex эту] прошивку, для mega128 и maga128A [https://er9x-frsky-mavlink.googlecode.com/svn/trunk/bin/er9x-128.hex эту] и для [https://er9x-frsky-mavlink.googlecode.com/svn/trunk/bin/er9x-2561.hex mega2561]
Перед прошивкой лучше убедиться, что скачанный файл не содержит html-код и т.п. В случае затруднений в скачивании используйте svn (описано ниже в вопросах и ответах) либо используйте прикрепленный файл в наиболее поздних постах в [http://forum.rcdesign.ru/f4/thread364947.html этой] теме на форуме.

6. Восстанавливаем настройки аппы из файла, созданного в п.4 (Write Memory To TX)

7. Владельцы mega64 этот пункт пропускают, а остальные в настройках аппы идут в Radio Setup (страница 1, в самом низу) "Frsky Mod Done" выставляем в "ON"

8. На аппе, в настройках модели, на странице 10 Telemetry в первой строке выставляем "Proto FrHub Mav Met"

9. Идем в просмотр телеметрии и видим, что появилась новая страница № 5 c "радаром" в центре. 

10. Можно выполнить снова п.4 чтобы после очередной перепрошивки не делать п.7 и п.8. На этом с аппой закончили.

11. Прошиваем ардуину 
  * Способ 1. Собираем прошивку в Arduino IDE из исходных кодов и прошиваем (более подробно процесс описан ниже в вопросах и ответах)
  * Способ 2. [https://er9x-frsky-mavlink.googlecode.com/svn/trunk/bin/APM_Mavlink_to_FrSky.hex этой] прошивкой с помощью USB-TTL конвертера (убедитесь в наличии линии DTR) и программы [http://russemotto.com/xloader/ XLoader]
  Если вы используете openLRSng, то в файле APM_Mavlink_to_FrSky.ino в строке 125, необходимо заменить строку "frSkySerial = new SoftwareSerial(6, 5, true);" на "frSkySerial = new SoftwareSerial(6, 5, false);"

12. Подключаем RX, TX, VCC, GND ардуины к разъему телеметрии APM

13. Подключаем D5 ардуины к разъему RX приемника (см. рисунок в начале этой страницы)

14. Если приемник и ардуина запитаны от разных источников, то соединяем GND ардуины и приемника

15. Отключаем от ардуины USB-TTL конвертер

16. Подключаем APM к аккумулятору, отключаем USB от APM (при подключенном USB телеметрия не передается)

17. Ждем около минуты пока прогрузится APM и ардуина

18. На аппе, на 5 экране телеметрии видим полетный режим и остальные параметры

19. Profit!

=Вопросы и ответы=

==Вопрос: Как прошить arduino ?==
Ответ: Существует два способа.

Первый способ - из исходных кодов

1. Качаем [http://arduino.cc/en/Main/Software Arduino IDE 1.0.6]

2. Качаем [https://code.google.com/p/er9x-frsky-mavlink/source/browse/trunk/?r=17#trunk%2Fsource%2Fmavlink-driver исходники], нас интересует папка "mavlink-driver". Для скачивания исходников лучше воспользоваться svn (описание ниже)

3. Заходим в IDE выбираем "Скетч" - "Импортировать библиотеку" - "Add library"

4. Выбираем папку mavlink-driver\libraries\AP_Common\ и далее повторяем с каждой папкой из mavlink-driver\libraries

5. Далее "Файл" - "Открыть" - "APM_Mavlink_to_FrSky.ino"

6. Подключаем Arduino и USB-TTL конвертер к порту USB

7. "Сервис" - "Плата" Выбираем нашу ардуину и "Сервис" - "Порт" - выбираем порт

8. В Arduino IDE в панели инструментов жмете вторую кнопку слева (выглядит как стрелка вправо) - все компилируется и заливается в ардуину

Второй способ - из готового hex файла.

Берем [http://russemotto.com/xloader/ XLoader] или Arduino Hex Uploader and Programmer и шьем [https://er9x-frsky-mavlink.googlecode.com/svn/trunk/bin/APM_Mavlink_to_FrSky.hex APM_Mavlink_to_FrSky.hex], через USB-TTL конвертер.

Прочитайте [http://www.getchip.net/posts/104-proshivka-lyubogo-hex-fajjla-v-arduino-pri-pomoshhi-shtatnogo-zagruzchika-bootloader/ это] (раздел про XLoader)

==Вопрос: Как скачать исходники?==
Ответ:

1. Устанавливаете любой SVN клиент, например [http://www.sliksvn.com/en/download этот]

2. В командной строке "cd \" - переходим в корень диска

3. В командной строке "svn checkout http://er9x-frsky-mavlink.googlecode.com/svn/trunk/ er9x-frsky-mavlink"

В корне диска появится папка "er9x-frsky-mavlink" с исходниками

==Вопрос: Чем отличается эта прошивка от обычной er9x?==
Ответ: это обычная r812 прошивка + 5 экран телеметрии + доп. настройка MAV-STD на 10 экране настройки модели.

В режиме Mav включается 5 экран телеметрии, в режиме Std - это полностью r812 er9x

==Вопрос: Чем отличаются прошивки между собой?==
Ответ: Версия для mega64 - урезанная.

Из стандартной 812 прошивки полностью убраны HELI и TEMPLATES, в моих доработках все направления считаются с высокой погрешностью, отключена индикация "здоровья" сенсоров, полностью отключена система передачи сообщений (PreArm и остальные)

mega128 и mega2561 идентичны (по возможностям)


==Вопрос: Как реализовано управление подсветкой?==
Ответ: Если вам нужно только управление подсветкой, то просто подключите ардуину, прошитую нашей прошивкой к порту телеметрии и ваш квадрик будет мигать в зависимости от полетного режима.

ВНИМАНИЕ!

Обычные светодиоды можно подключать к ардуине только через токоограничивающие резисторы, а яркие светодиоды, светодиодные ленты и т.п. напрямую к ардуине подключать нельзя (ардуина выдерживает ток только до 20 миллиампер на один контакт и 200 на все устройство). За консультацией по подключению ярких светодиодов и лент к ардуине обращайтесь к спецам по электронике.

4 канала подсветки на лучи: FRONT (arduino pin 7), REAR - 8, LEFT - 9, RIGHT - 10.

Подсветка настраивается по "паттернам" (об этом ниже)

4 канала для цветных светодиодов вроде [http://ru.aliexpress.com/premium/3W-4LEDs-APM.html?ltype=wholesale&SearchText=3W+4LEDs+APM&d=y&origin=y&initiative_id=SB_20141005022206&catId=&isViewCP=y&LocalSearchText=3W+4LEDs+APM&enSearchText=3W+4LEDs+APM этих]

Белый светодиод (pin 2) включается при арминге

Голубой светодиод (pin 3) включается при 3D Fix и hdop <= 200

Красный светодиод (pin 4) включается при получении любого сообщения от APM уровня CRITICAL

Зеленый светодиод (pin 13) и светодиод на ардуине повторяют светодиоды переднего луча

По паттернам. Подсветка настраивается только в исходниках, настраивать через специальную программу как в jd IO_Board возможности нет, PWM тоже не реализовано (т.е. нельзя плавно включать-выключать светодиоды)

В файле APM_Mavlink_to_FrSky.ino начиная где-то со строки 60 вы найдете код:

{{{
char LEFT_STAB[]   PROGMEM = "1111111110";   // pattern for LEFT  light, mode - STAB
char RIGHT_STAB[]  PROGMEM = "1111111110";   // pattern for RIGHT light, mode - STAB
char FRONT_STAB[]  PROGMEM = "1111111110";   // pattern for FRONT light, mode - STAB
char REAR_STAB[]   PROGMEM = "1111111110";   // pattern for REAR  light, mode - STAB

char LEFT_AHOLD[]  PROGMEM = "111000";  // medium blink
char RIGHT_AHOLD[] PROGMEM = "111000"; 
char FRONT_AHOLD[] PROGMEM = "111000"; 
char REAR_AHOLD[]  PROGMEM = "111000"; 

char LEFT_RTL[]    PROGMEM = "10";  // fast blink
char RIGHT_RTL[]   PROGMEM = "10"; 
char FRONT_RTL[]   PROGMEM = "10"; 
char REAR_RTL[]    PROGMEM = "10"; 

char LEFT_OTHER[]  PROGMEM = "1";  // always ON
char RIGHT_OTHER[] PROGMEM = "1"; 
char FRONT_OTHER[] PROGMEM = "1"; 
char REAR_OTHER[]  PROGMEM = "1";
}}}


Первая строка - это настройка подсветки левого луча в режиме STABILIZE. Один символ в строке - это 0,1сек времени. По умолчанию сейчас 0,9сек подсветка горит, затем на 0,1сек тухнет для всех лучей. Соответственно, строка типа "1111100000" будет означать 0,5сек вкл затем 0,5сек выкл, строка "01" 0,1сек выкл затем 0,1сек вкл и т.д. Строки могут быть разной длины. Суммарная длина всех таких строк ограничена размером флеш памяти на ардуине. Для подсветки осталось около 500 байт - довольно много.

Сейчас реализована настройка подсветки в четырех режимах: stab, althold, rtl и все остальные. Каждый из четырех лучей настраивается индивидуально.


Подсветка по умолчанию сейчас: STAB - медленно мигает (0.9+0.1),

ALTHOLD - средне (0,3+0,3), RTL - быстро (0,1+0,1), на остальных режимах горит постоянно.

Настройка на большее количество полетных режимов описана [http://forum.rcdesign.ru/f4/thread364947-5.html#post5309013 тут]

==Вопрос: Какие особенности при использовании оборудования openLRSng?==
Ответ: Приемник FrSky использует инвертированный TTL сигнал UART, а приемники openLRSng - обычный, поэтому необходимо внесение минимальных изменений в код Arduino. Смотрите п.11 инструкции.
