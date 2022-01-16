#ifndef FMSYNHT_H
#define FMSYNHT_H

#include "pluginutil.h"

#define FM "fm"
#define MOD "M"
#define ENV "env"

#define NOMINATOR FM "r1"
#define DIVIDER FM "r2"
#define WAVEFORM FM "wav"
#define VEL_SENS FM "vel"

#define OUT MOD "out"
#define ROUTE(i) \
    (i == 0 ? OUT : (i == 1 ? MOD "o1" : (i == 2 ? MOD "o2" : (i == 3 ? MOD "o3" : MOD "o4"))))

#define A ENV "A"
#define D ENV "D"
#define S ENV "S"
#define R ENV "R"

double *get_fmsynth_param(int osc, const char *name)
{
    char buf[64];
    sprintf(buf, "o%d%s", osc, name);
    Parameter *p = get_parameter_ptr(buf);
    return p ? &p->value : &parameters[LAST_PARAM].value;
}

#define POSC(osc, name) get_fmsynth_param(osc, name)

double *get_by_route(const char *route)
{
    // a-b
    int a = route[0] - '0';
    int b = route[2];
    if (b == 'o')
        return POSC(a, OUT);

    b -= '0';
    LOG("Get route %s = o%d%s", route, a, ROUTE(b));
    return POSC(a, ROUTE(b));
}

int is_route_active(const char *route)
{
    return *get_by_route(route) > 1e-6;
}

#endif