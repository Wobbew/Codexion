#include "codexion.h"
int ft_isnumeric(char *s)
{
    int i = 0;
    if (s[i] == '+' || s[i] == '-')
        i++;
    while (s[i])
    {
        if (s[i] < '0' || s[i] > '9')
            return 0;
        i++;
    }
    return 1;
}

bool input_error_schedul(void)
{
    fprintf(stderr, "Invalid scheduler value provided. Please use 'fifo' or 'edf'.\n");
    return false;
}

bool input_error(void)
{
    fprintf(stderr, "Usage: codexion <num_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <num_compiles_required> <dongle_cooldown> <scheduler>\n");
    return false;
}
bool input_error_int(void)
{
    fprintf(stderr, "Invalid integer value provided.\n");
    return false;
}
bool test_args(t_params *params)
{
    if (params->num_coders < 1)
        return (input_error_int());
    if (params->time_to_burnout < 0)
        return (input_error_int());
    if (params->time_to_compile < 0)
        return (input_error_int());
    if (params->time_to_debug < 0)
        return (input_error_int());
    if (params->time_to_refactor < 0)
        return (input_error_int());
    if (params->num_compiles_required < 1)
        return (input_error_int());
    if (params->dongle_cooldown < 0)
        return (input_error_int());
    if (strcmp(params->scheduler, "fifo") != 0 && strcmp(params->scheduler, "edf") != 0)
        return (input_error_schedul());
    return true;
}


bool params_args(int argc, char **argv, t_params *params)
{
    if (argc != 9)
        return input_error();
    for (int i = 1; i < argc-1 ; i++)
    {
        if (ft_isnumeric(argv[i]) == 0)
            return (input_error_int());
    }
    params->num_coders = atoi(argv[1]);
    params->time_to_burnout = atoi(argv[2]);
    params->time_to_compile = atoi(argv[3]);
    params->time_to_debug = atoi(argv[4]);
    params->time_to_refactor = atoi(argv[5]);
    params->num_compiles_required = atoi(argv[6]);
    params->dongle_cooldown = atoi(argv[7]);
    params->scheduler = argv[8];
    if (test_args(params) != true)
        return false;
    return true;
}

bool setup(t_sim *sim)
{
    if (setup_sim(sim) != true)
        return false;
    if (setup_dongles(sim) != true)
        return false;
    if (setup_coders(sim) != true)
        return false;
    return true;
}
bool setup_sim(t_sim *sim)
{    
    sim->coders = malloc(sizeof(t_coder) * sim->params.num_coders);
    if (!sim->coders)
        return false;
    sim->dongles = malloc(sim->params.num_coders * sizeof(t_dongle));
    if (!sim->dongles)
        return free(sim->coders), false;
    sim->start_ms = get_time_ms();
    pthread_mutex_init(&sim->log_mutex, NULL);
    pthread_mutex_init(&sim->stop_mutex, NULL);
    sim->stopped = false;
    return true;
}
void log_state(t_sim *sim, int id, const char *msg)
{
    pthread_mutex_lock(&sim->log_mutex);
    printf("%ld %d %s\n", get_time_ms() - sim->start_ms, id, msg);
    pthread_mutex_unlock(&sim->log_mutex);
}
static void acquire_dongle(t_dongle *dongle, long cooldown)
{
    pthread_mutex_lock(&dongle->mutex);
    while (!dongle->available || get_time_ms() - dongle->released_at_ms < cooldown)
        pthread_cond_wait(&dongle->cond, &dongle->mutex);
    dongle->available = false;
    pthread_mutex_unlock(&dongle->mutex);
}
static void get_dongle(t_coder *coder)
{
    long cooldown = coder->sim->params.dongle_cooldown;
    if (coder->dongle_left->id < coder->dongle_right->id)
    {
        acquire_dongle(coder->dongle_left, cooldown);
        log_state(coder->sim, coder->id, "has taken a dongle");
        acquire_dongle(coder->dongle_right, cooldown);
        log_state(coder->sim, coder->id, "has taken a dongle");
    }
    else
    {
        acquire_dongle(coder->dongle_right, cooldown);
        log_state(coder->sim, coder->id, "has taken a dongle");
        acquire_dongle(coder->dongle_left, cooldown);
        log_state(coder->sim, coder->id, "has taken a dongle");
    }
}
static void release_dongle(t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->mutex);
    dongle->available = true;
    dongle->released_at_ms = get_time_ms();
    pthread_cond_broadcast(&dongle->cond);
    pthread_mutex_unlock(&dongle->mutex);
}

static void act(t_coder *coder)
{
    coder->last_compile_ms = get_time_ms();
    log_state(coder->sim, coder->id, "is compiling");
    usleep(coder->sim->params.time_to_compile * 1000);
    coder->compile_count++;
    release_dongle(coder->dongle_left);
    release_dongle(coder->dongle_right);
    log_state(coder->sim, coder->id, "is debugging");
    usleep(coder->sim->params.time_to_debug * 1000);
    log_state(coder->sim, coder->id, "is refactoring");
    usleep(coder->sim->params.time_to_refactor * 1000);

}

void *coder_routine(void *arg)
{
    t_coder *coder = (t_coder *)arg;
    while (true)
    {
        if (coder->sim->stopped)
            break;
        get_dongle(coder);
        act(coder);
        if (coder->compile_count >= coder->sim->params.num_compiles_required)
            break;
    }
    
    return NULL;
}
bool setup_coders(t_sim *sim)
{
    for (int i = 0; i < sim->params.num_coders; i++)
    {
        sim->coders[i].id = i+1;
        sim->coders[i].compile_count = 0;
        sim->coders[i].last_compile_ms = sim->start_ms;
        sim->coders[i].dongle_right = &sim->dongles[i];
        sim->coders[i].dongle_left = &sim->dongles[(i + 1) % sim->params.num_coders];
        sim->coders[i].sim = sim;
        if (pthread_create(&sim->coders[i].thread, NULL, coder_routine, &sim->coders[i]) != 0)
	    return (false);
    }
    return true;
}
bool setup_dongles(t_sim *sim)
{
    for (int i = 0; i < sim->params.num_coders; i++)
    {
        sim->dongles[i].id = i+1;
        sim->dongles[i].available = true;
        sim->dongles[i].released_at_ms = 0;
        pthread_mutex_init(&sim->dongles[i].mutex, NULL);
        pthread_cond_init(&sim->dongles[i].cond, NULL);
    }
    return true;
}

long    get_time_ms(void)
{
    struct timeval  tv;

    gettimeofday(&tv, NULL);
    return ((long)tv.tv_sec * 1000L + (long)tv.tv_usec / 1000L);
}
void *monitor_routine(void *arg)
{
    t_sim *sim = (t_sim *)arg;
    while (sim->stopped == false)
    {
    int compile_count;
    compile_count = 0;
    for (int i = 0; i < sim->params.num_coders; i++)
    {
        if (get_time_ms() - sim->coders[i].last_compile_ms > sim->params.time_to_burnout)
        {
            log_state(sim, sim->coders[i].id, "burned out");
            sim->stopped = true;
            return NULL;
        }
        compile_count += sim->coders[i].compile_count;
    }
    if (compile_count >= sim->params.num_compiles_required* sim->params.num_coders)
    {
        sim->stopped = true;
        return NULL;
    }
    usleep(10000);
    }
    return NULL;
}
void join_threads(t_sim *sim)
{
    for (int i = 0; i < sim->params.num_coders; i++)
    {
        pthread_join(sim->coders[i].thread, NULL);
    }
    pthread_join(sim->monitor, NULL);
}
int main(int argc, char **argv)
{
    t_params params;
    t_sim  sim;
    if (params_args(argc, argv, &params) != true)
        return 1;
    sim.params = params;
    if (setup(&sim) != true)
        return 1;
    pthread_create(&sim.monitor, NULL, monitor_routine, &sim);
    join_threads(&sim);
    return 0;
}
