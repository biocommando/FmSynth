#define LOGGING
//#define LOG_LEVEL LOG_LEVEL_TRACE
#include "fmsynth.h"
#include "static_keyval_map.h"

STATIC_KV_MAP initial_routes;

double route_amount = 0.5;

void set_all_routes_to_zero()
{
    initial_routes = static_kv_map_create();
    for (int osc = 1; osc <= 4; osc++)
        for (int route = 0; route <= 4; route++)
        {
            char route_name[10];
            sprintf(route_name, "%d-%c", osc, (route == 0 ? 'o' : '0' + route));
            double *route_p = POSC(osc, ROUTE(route));
            LOG("Set initial route value %s = %lf", route_name, *route_p);
            static_kv_map_set(route_name, *route_p, &initial_routes);
            *route_p = 0;
        }
}

void enable_route(const char *route) {
    char route_cpy[10];
    strcpy(route_cpy, route);
    route_cpy[3] = 0;
    double initial_route_amount = static_kv_map_get(route_cpy, &initial_routes);
    LOG("Read initial route value %s = %lf", route_cpy, initial_route_amount);
    if (initial_route_amount > 1e-6)
    {
        *get_by_route(route_cpy) = initial_route_amount;
        return;
    }
    *get_by_route(route_cpy) = route_amount;
}

/*
    Selects one pre-selected of the algorithms. Similar to DX100
    in functionality. Algos:

    1:
        4-4
        4-3-2-1-out

        unique: not(4,5,6,7,8) & 4-3
    2:
        4-4
        4-2
        3-2
        2-1-out

        unique: not(4,5,6,7,8) & 4-2
    3:
        4-4
        4-1
        3-2-1
        1-out

        unique: not(4,5,6,7,8) & 4-1
    4:
        4-4
        3-1
        2-1
        1-out

        unique: 3-1
    5:
        4-4
        4-3-out
        2-1-out

        unique: 1-out & 3-out only
    6:
        4-4
        4-1-out
        4-2-out
        4-3-out

        unique: 4-all
    7:
        4-4
        4-3-out
        2-out
        1-out

        unique: 1-out & 2-out & 3-out only & not(6)
    8:
        4-4
        4-out
        3-out
        2-out
        1-out

        unique: 4-out
*/
void select_algorithm(int algo)
{
    LOG("selecting algo %d", algo);
    set_all_routes_to_zero();
    *get_by_route("4-4") = route_amount * 0.1;
    switch (algo)
    {
    case 1:
        enable_route("4-3");
        enable_route("3-2");
        enable_route("2-1");
        enable_route("1-out");
        break;

    case 2:
        enable_route("4-2");
        enable_route("3-2");
        enable_route("2-1");
        enable_route("1-out");
        break;

    case 3:
        enable_route("4-1");
        enable_route("3-2");
        enable_route("2-1");
        enable_route("1-out");
        break;

    case 4:
        enable_route("4-3");
        enable_route("3-1");
        enable_route("2-1");
        enable_route("1-out");
        break;

    case 5:
        enable_route("4-3");
        enable_route("3-out");
        enable_route("2-1");
        enable_route("1-out");
        break;

    case 6:
        enable_route("4-3");
        enable_route("4-2");
        enable_route("4-1");
        enable_route("3-out");
        enable_route("2-out");
        enable_route("1-out");
        break;

    case 7:
        enable_route("4-3");
        enable_route("3-out");
        enable_route("2-out");
        enable_route("1-out");
        break;

    case 8:
        enable_route("4-out");
        enable_route("3-out");
        enable_route("2-out");
        enable_route("1-out");
        break;

    default:
        break;
    }
}

int detect_algorithm()
{
    if (is_route_active("3-1"))
        return 4;
    if (is_route_active("4-out"))
        return 8;
    if (is_route_active("4-3") && is_route_active("4-2") && is_route_active("4-1"))
        return 6;
    if (is_route_active("4-1"))
        return 3;
    if (is_route_active("2-out"))
        return 7;
    if (is_route_active("3-out"))
        return 5;
    if (is_route_active("4-2"))
        return 2;
    return 1;
}

void cycle_algorithm(int cycle_dir)
{
    int detected_algo = detect_algorithm();
    LOG("Detected algo: %d", detected_algo);
    int select_algo = detected_algo + cycle_dir;
    if (select_algo == 0) select_algo = 8;
    if (select_algo == 9) select_algo = 1;
    select_algorithm(select_algo);
}

PLUGIN()
{
    route_amount = P(route_amount);
    const double algo = P(algo);
    if (algo > 1e-6)
        select_algorithm(algo);
    else
        cycle_algorithm(P(cycle_dir));
}