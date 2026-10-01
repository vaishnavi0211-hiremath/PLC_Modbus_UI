#include "main.h"

#include <slint.h>
#include <modbus/modbus.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

const char *PLC_IP = "192.168.0.1";
const int PLC_PORT = 502;
const int SLAVE_ID = 1;


// ============================================================
// GET CURRENT TIME
// ============================================================

std::string get_current_time()
{
    auto now = std::chrono::system_clock::now();

    std::time_t time_now =
        std::chrono::system_clock::to_time_t(now);

    std::tm local_time{};

#ifdef _WIN32

    localtime_s(&local_time, &time_now);

#else

    localtime_r(&time_now, &local_time);

#endif

    std::ostringstream stream;

    stream << std::put_time(
        &local_time,
        "%H:%M:%S"
    );

    return stream.str();
}


// ============================================================
// PRINT PLC CONNECTED
// ============================================================

void print_connected()
{
    std::cout
        << "["
        << get_current_time()
        << "] PLC CONNECTED - "
        << PLC_IP
        << ":"
        << PLC_PORT
        << std::endl;
}


// ============================================================
// PRINT PLC DISCONNECTED
// ============================================================

void print_disconnected()
{
    std::cout
        << "["
        << get_current_time()
        << "] PLC DISCONNECTED -"
        << PLC_IP
        << ":"
        << PLC_PORT
        << std::endl;
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    // ========================================================
    // CREATE UI
    // ========================================================

    auto ui = MainWindow::create();


    // ========================================================
    // INITIAL UI
    // ========================================================

    ui->set_plc_ip(
        slint::SharedString(PLC_IP)
    );

    ui->set_plc_port(
        slint::SharedString(
            std::to_string(PLC_PORT)
        )
    );

    // Initially PLC is not connected.
    // IMPORTANT:
    // We do NOT show CONNECTING or ATTEMPTING.

    ui->set_plc_online(false);

    ui->set_register0_on(false);

    ui->set_register1_on(false);

    ui->set_register2_bit2(false);

    ui->set_latency(
        slint::SharedString("-- ms")
    );

    ui->set_response(
        slint::SharedString("DISCONNECTED")
    );


    // ========================================================
    // CREATE MODBUS CONTEXT
    // ========================================================

    modbus_t *ctx =
        modbus_new_tcp(
            PLC_IP,
            PLC_PORT
        );


    if (ctx == nullptr)
    {
        std::cerr
            << "["
            << get_current_time()
            << "] Failed to create Modbus context"
            << std::endl;

        return 1;
    }


    // ========================================================
    // SET SLAVE ID
    // ========================================================

    if (
        modbus_set_slave(
            ctx,
            SLAVE_ID
        ) == -1
    )
    {
        std::cerr
            << "["
            << get_current_time()
            << "] Failed to set slave ID: "
            << modbus_strerror(errno)
            << std::endl;

        modbus_free(ctx);

        return 1;
    }


    // ========================================================
    // MODBUS RESPONSE TIMEOUT
    // ========================================================
    //
    // 300 ms timeout.
    //
    // If PLC is unplugged/disconnected, the read will fail
    // quickly and the application will report DISCONNECTED.
    //

    modbus_set_response_timeout(
        ctx,
        0,
        300000
    );


    // ========================================================
    // THREAD CONTROL
    // ========================================================

    std::mutex modbus_mutex;

    std::atomic<bool> running(true);

    std::atomic<bool> set_register0(false);

    std::atomic<bool> set_register1(false);


    // ========================================================
    // BUTTON 1
    // REGISTER 0 BIT 0
    // ========================================================

    ui->on_set_button_clicked(
        [&set_register0]()
        {
            set_register0.store(true);
        }
    );


    // ========================================================
    // BUTTON 2
    // REGISTER 1 BIT 1
    // ========================================================

    ui->on_second_button_clicked(
        [&set_register1]()
        {
            set_register1.store(true);
        }
    );


    // ========================================================
    // MODBUS WORKER THREAD
    // ========================================================

    std::thread modbus_thread(
        [&]()
        {
            bool connected = false;


            // ====================================================
            // HELPER:
            // MARK PLC DISCONNECTED
            // ====================================================

            auto mark_disconnected =
                [&]()
                {
                    if (connected)
                    {
                        connected = false;

                        print_disconnected();
                    }

                    slint::invoke_from_event_loop(
                        [ui]()
                        {
                            ui->set_plc_online(false);

                            ui->set_register0_on(false);

                            ui->set_register1_on(false);

                            ui->set_register2_bit2(false);

                            ui->set_latency(
                                slint::SharedString(
                                    "-- ms"
                                )
                            );

                            ui->set_response(
                                slint::SharedString(
                                    "DISCONNECTED"
                                )
                            );
                        }
                    );
                };


            // ====================================================
            // WORKER LOOP
            // ====================================================

            while (running.load())
            {

                // =================================================
                // AUTO RECONNECT
                // =================================================
                //
                // IMPORTANT:
                // There is NO "CONNECTING" status.
                //
                // The application silently tries to reconnect.
                //
                // Only successful connection is displayed.
                //

                if (!connected)
                {
                    bool connection_successful = false;


                    {
                        std::lock_guard<std::mutex> lock(
                            modbus_mutex
                        );


                        if (
                            modbus_connect(ctx) == 0
                        )
                        {
                            connection_successful = true;
                        }
                    }


                    if (connection_successful)
                    {
                        connected = true;


                        // -----------------------------------------
                        // CONSOLE
                        // -----------------------------------------

                        print_connected();


                        // -----------------------------------------
                        // UI
                        // -----------------------------------------

                        slint::invoke_from_event_loop(
                            [ui]()
                            {
                                ui->set_plc_online(true);

                                ui->set_response(
                                    slint::SharedString(
                                        "CONNECTED"
                                    )
                                );
                            }
                        );
                    }
                    else
                    {
                        // -----------------------------------------
                        // DO NOT SHOW "CONNECTING"
                        //
                        // Simply wait and try again.
                        // -----------------------------------------

                        std::this_thread::sleep_for(
                            std::chrono::milliseconds(500)
                        );

                        continue;
                    }
                }


                // =================================================
                // REGISTER 0 BIT 0
                //
                // SET -> WAIT 1 SEC -> RESET
                // =================================================

                if (
                    set_register0.exchange(false)
                )
                {
                    std::lock_guard<std::mutex> lock(
                        modbus_mutex
                    );


                    uint16_t value = 0;


                    // ---------------------------------------------
                    // READ REGISTER 0
                    // ---------------------------------------------

                    int result =
                        modbus_read_registers(
                            ctx,
                            0,
                            1,
                            &value
                        );


                    if (result == -1)
                    {
                        modbus_close(ctx);

                        mark_disconnected();

                        continue;
                    }


                    // ---------------------------------------------
                    // SET BIT 0
                    // ---------------------------------------------

                    value |= (1 << 0);


                    result =
                        modbus_write_register(
                            ctx,
                            0,
                            value
                        );


                    if (result == -1)
                    {
                        modbus_close(ctx);

                        mark_disconnected();

                        continue;
                    }


                    // ---------------------------------------------
                    // UI - BIT 0 ON
                    // ---------------------------------------------

                    slint::invoke_from_event_loop(
                        [ui]()
                        {
                            ui->set_register0_on(true);

                            ui->set_response(
                                slint::SharedString(
                                    "CONNECTED"
                                )
                            );
                        }
                    );


                    // ---------------------------------------------
                    // KEEP ON FOR 1 SECOND
                    // ---------------------------------------------

                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(1000)
                    );


                    if (!connected)
                    {
                        continue;
                    }


                    // ---------------------------------------------
                    // READ REGISTER AGAIN
                    // ---------------------------------------------

                    result =
                        modbus_read_registers(
                            ctx,
                            0,
                            1,
                            &value
                        );


                    if (result == -1)
                    {
                        modbus_close(ctx);

                        mark_disconnected();

                        continue;
                    }


                    // ---------------------------------------------
                    // RESET BIT 0
                    // ---------------------------------------------

                    value &= ~(1 << 0);


                    result =
                        modbus_write_register(
                            ctx,
                            0,
                            value
                        );


                    if (result == -1)
                    {
                        modbus_close(ctx);

                        mark_disconnected();

                        continue;
                    }


                    // ---------------------------------------------
                    // UI - BIT 0 OFF
                    // ---------------------------------------------

                    slint::invoke_from_event_loop(
                        [ui]()
                        {
                            ui->set_register0_on(false);
                        }
                    );
                }


                // =================================================
                // REGISTER 1 BIT 1
                //
                // SET -> WAIT 1 SEC -> RESET
                // =================================================

                if (
                    set_register1.exchange(false)
                )
                {
                    std::lock_guard<std::mutex> lock(
                        modbus_mutex
                    );


                    uint16_t value = 0;


                    // ---------------------------------------------
                    // READ REGISTER 1
                    // ---------------------------------------------

                    int result =
                        modbus_read_registers(
                            ctx,
                            1,
                            1,
                            &value
                        );


                    if (result == -1)
                    {
                        modbus_close(ctx);

                        mark_disconnected();

                        continue;
                    }


                    // ---------------------------------------------
                    // SET BIT 1
                    // ---------------------------------------------

                    value |= (1 << 1);


                    result =
                        modbus_write_register(
                            ctx,
                            1,
                            value
                        );


                    if (result == -1)
                    {
                        modbus_close(ctx);

                        mark_disconnected();

                        continue;
                    }


                    // ---------------------------------------------
                    // UI - BIT 1 ON
                    // ---------------------------------------------

                    slint::invoke_from_event_loop(
                        [ui]()
                        {
                            ui->set_register1_on(true);

                            ui->set_response(
                                slint::SharedString(
                                    "CONNECTED"
                                )
                            );
                        }
                    );


                    // ---------------------------------------------
                    // KEEP ON FOR 1 SECOND
                    // ---------------------------------------------

                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(1000)
                    );


                    if (!connected)
                    {
                        continue;
                    }


                    // ---------------------------------------------
                    // READ REGISTER AGAIN
                    // ---------------------------------------------

                    result =
                        modbus_read_registers(
                            ctx,
                            1,
                            1,
                            &value
                        );


                    if (result == -1)
                    {
                        modbus_close(ctx);

                        mark_disconnected();

                        continue;
                    }


                    // ---------------------------------------------
                    // RESET BIT 1
                    // ---------------------------------------------

                    value &= ~(1 << 1);


                    result =
                        modbus_write_register(
                            ctx,
                            1,
                            value
                        );


                    if (result == -1)
                    {
                        modbus_close(ctx);

                        mark_disconnected();

                        continue;
                    }


                    // ---------------------------------------------
                    // UI - BIT 1 OFF
                    // ---------------------------------------------

                    slint::invoke_from_event_loop(
                        [ui]()
                        {
                            ui->set_register1_on(false);
                        }
                    );
                }


                // =================================================
                // MONITOR REGISTER 2 BIT 2
                //
                // THIS ALSO CHECKS PLC CONNECTION.
                // =================================================

                {
                    std::lock_guard<std::mutex> lock(
                        modbus_mutex
                    );


                    uint16_t value = 0;


                    auto start =
                        std::chrono::steady_clock::now();


                    int result =
                        modbus_read_registers(
                            ctx,
                            2,
                            1,
                            &value
                        );

                    auto end =
                        std::chrono::steady_clock::now();


                    double latency =
                        std::chrono::duration<double, std::milli>(
                            end - start
                        ).count();


                    // ---------------------------------------------
                    // PLC DISCONNECTED
                    // ---------------------------------------------

                    if (result == -1)
                    {
                        modbus_close(ctx);

                        mark_disconnected();

                        continue;
                    }


                    // ---------------------------------------------
                    // REGISTER 2 BIT 2
                    // ---------------------------------------------

                    bool bit2 =
                        (value & (1 << 2)) != 0;


                    // ---------------------------------------------
                    // LATENCY
                    // ---------------------------------------------

                    char latency_text[64];


                    std::snprintf(
                        latency_text,
                        sizeof(latency_text),
                        "%.2f ms",
                        latency
                    );


                    std::string latency_string =
                        latency_text;


                    // ---------------------------------------------
                    // UI
                    // ---------------------------------------------

                    slint::invoke_from_event_loop(
                        [
                            ui,
                            bit2,
                            latency_string
                        ]()
                        {
                            ui->set_plc_online(true);

                            ui->set_register2_bit2(
                                bit2
                            );

                            ui->set_latency(
                                slint::SharedString(
                                    latency_string
                                )
                            );

                            ui->set_response(
                                slint::SharedString(
                                    "CONNECTED"
                                )
                            );
                        }
                    );
                }


                // =================================================
                // POLL EVERY 200 ms
                // =================================================

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(200)
                );
            }


            // ====================================================
            // THREAD EXIT
            // ====================================================

            if (connected)
            {
                modbus_close(ctx);
            }
        }
    );


    // ============================================================
    // RUN SLINT
    // ============================================================

    ui->run();


    // ============================================================
    // STOP THREAD
    // ============================================================

    running.store(false);


    // ============================================================
    // WAIT FOR THREAD
    // ============================================================

    if (modbus_thread.joinable())
    {
        modbus_thread.join();
    }


    // ============================================================
    // FREE MODBUS
    // ============================================================

    modbus_free(ctx);


    std::cout
        << "["
        << get_current_time()
        << "] Application closed"
        << std::endl;


    return 0;
}