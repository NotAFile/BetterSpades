#include "common.h"

unsigned char sound_enabled = 1;

struct Sound_source sound_sources[SOUND_VOICES_MAX];
unsigned char sound_sources_free[SOUND_VOICES_MAX];

struct Sound_wav sound_footstep1;
struct Sound_wav sound_footstep2;
struct Sound_wav sound_footstep3;
struct Sound_wav sound_footstep4;

struct Sound_wav sound_wade1;
struct Sound_wav sound_wade2;
struct Sound_wav sound_wade3;
struct Sound_wav sound_wade4;

struct Sound_wav sound_jump;
struct Sound_wav sound_jump_water;

struct Sound_wav sound_land;
struct Sound_wav sound_land_water;

struct Sound_wav sound_hurt_fall;

struct Sound_wav sound_explode;
struct Sound_wav sound_explode_water;
struct Sound_wav sound_grenade_bounce;
struct Sound_wav sound_grenade_pin;

struct Sound_wav sound_pickup;
struct Sound_wav sound_horn;

struct Sound_wav sound_rifle_shoot;
struct Sound_wav sound_rifle_reload;
struct Sound_wav sound_smg_shoot;
struct Sound_wav sound_smg_reload;
struct Sound_wav sound_shotgun_shoot;
struct Sound_wav sound_shotgun_reload;
struct Sound_wav sound_shotgun_cock;

struct Sound_wav sound_hitground;
struct Sound_wav sound_hitplayer;
struct Sound_wav sound_build;

struct Sound_wav sound_spade_woosh;
struct Sound_wav sound_spade_whack;

struct Sound_wav sound_death;
struct Sound_wav sound_beep1;
struct Sound_wav sound_beep2;
struct Sound_wav sound_switch;
struct Sound_wav sound_empty;
struct Sound_wav sound_intro;

struct Sound_wav sound_debris;
struct Sound_wav sound_bounce;
struct Sound_wav sound_impact;


int sound_free_index() {
    for(int k=0;k<SOUND_VOICES_MAX;k++)
        if(sound_sources_free[k])
            return k;
    printf("Could not find free sound channel!\n");
    return 0;
}

struct Sound_source* sound_create(struct Sound_source* s, int option, struct Sound_wav* w, float x, float y, float z) {
    return sound_createEx(s,option,w,x,y,z,0.0F,0.0F,0.0F);
}


static struct Sound_source dummy;
struct Sound_source* sound_createEx(struct Sound_source* s, int option, struct Sound_wav* w, float x, float y, float z, float vx, float vy, float vz) {
    if(!sound_enabled)
        return &dummy;

    if(option==SOUND_WORLD && distance3D(camera_x,camera_y,camera_z,x,y,z)>128.0F*128.0F) {
        return &dummy;
    }

    int i = sound_free_index();
    sound_sources_free[i] = 0;

    sound_sources[i].local = option==SOUND_LOCAL;
    sound_sources[i].stick_to_player = -1;
    sound_sources[i].active = 1;

    alGenSources(1,&sound_sources[i].openal_handle);
    alSourcef(sound_sources[i].openal_handle,AL_PITCH,1.0F);
    alSourcef(sound_sources[i].openal_handle,AL_GAIN,1.0F);
    alSourcef(sound_sources[i].openal_handle,AL_REFERENCE_DISTANCE,(option==SOUND_LOCAL)?0.0F:w->min*SOUND_SCALE);
    alSourcef(sound_sources[i].openal_handle,AL_MAX_DISTANCE,(option==SOUND_LOCAL)?2048.0F:w->max*SOUND_SCALE);
    alSource3f(sound_sources[i].openal_handle,AL_POSITION,(option==SOUND_LOCAL)?0.0F:x*SOUND_SCALE,(option==SOUND_LOCAL)?0.0F:y*SOUND_SCALE,(option==SOUND_LOCAL)?0.0F:z*SOUND_SCALE);
    alSource3f(sound_sources[i].openal_handle,AL_VELOCITY,(option==SOUND_LOCAL)?0.0F:vx*SOUND_SCALE,(option==SOUND_LOCAL)?0.0F:vy*SOUND_SCALE,(option==SOUND_LOCAL)?0.0F:vz*SOUND_SCALE);
    alSourcei(sound_sources[i].openal_handle,AL_SOURCE_RELATIVE,(option==SOUND_LOCAL));
    alSourcei(sound_sources[i].openal_handle,AL_LOOPING,AL_FALSE);
    alSourcei(sound_sources[i].openal_handle,AL_BUFFER,w->openal_buffer);

    alSourcePlay(sound_sources[i].openal_handle);

    if(s!=NULL)
        memcpy(s,&sound_sources[i],sizeof(struct Sound_source));

    return &sound_sources[i];
}

void sound_velocity(struct Sound_source* s, float vx, float vy, float vz) {
    if(!sound_enabled || s->local)
        return;
    alSource3f(s->openal_handle,AL_VELOCITY,vx*SOUND_SCALE,vy*SOUND_SCALE,vz*SOUND_SCALE);
}

void sound_position(struct Sound_source* s, float x, float y, float z) {
    if(!sound_enabled || s->local)
        return;
    alSource3f(s->openal_handle,AL_POSITION,x*SOUND_SCALE,y*SOUND_SCALE,z*SOUND_SCALE);
}

void sound_update() {
    if(!sound_enabled)
        return;

    float orientation[] =   {sin(camera_rot_x)*sin(camera_rot_y),
                             cos(camera_rot_y),
                             cos(camera_rot_x)*sin(camera_rot_y),
                             0.0F,1.0F,0.0F};
    alListener3f(AL_POSITION,camera_x*SOUND_SCALE,camera_y*SOUND_SCALE,camera_z*SOUND_SCALE);
    alListener3f(AL_VELOCITY,camera_vx*SOUND_SCALE,camera_vy*SOUND_SCALE,camera_vz*SOUND_SCALE);
    alListenerfv(AL_ORIENTATION,orientation);

    for(int k=0;k<SOUND_VOICES_MAX;k++) {
        if(!sound_sources_free[k]) {
            int source_state;
            alGetSourcei(sound_sources[k].openal_handle,AL_SOURCE_STATE,&source_state);
            if(source_state==AL_STOPPED || (sound_sources[k].stick_to_player>=0 && !players[sound_sources[k].stick_to_player].connected)) {
                sound_sources[k].active = 0;
                alDeleteSources(1,&sound_sources[k].openal_handle);
                sound_sources_free[k] = 1;
            } else {
                if(sound_sources[k].stick_to_player>=0) {
                    sound_position(&sound_sources[k],
                        players[sound_sources[k].stick_to_player].pos.x,
                        players[sound_sources[k].stick_to_player].pos.y,
                        players[sound_sources[k].stick_to_player].pos.z);
                    sound_velocity(&sound_sources[k],
                        players[sound_sources[k].stick_to_player].physics.velocity.x,
                        players[sound_sources[k].stick_to_player].physics.velocity.y,
                        players[sound_sources[k].stick_to_player].physics.velocity.z);
                }
            }
        }
    }

}

extern short* drwav_open_and_read_file_s16(const char* filename, unsigned int* channels, unsigned int* sampleRate, uint64_t* totalSampleCount);

void sound_load(struct Sound_wav* wav, char* name, float min, float max) {
    if(!sound_enabled)
        return;
    unsigned int channels, samplerate;
    uint64_t samplecount;
    short* samples = drwav_open_and_read_file_s16(name,&channels,&samplerate,&samplecount);
    if(samples==NULL) {
        printf("Could not load sound %s\n",name);
        exit(1);
    }

    short* audio;
    if(channels>1) { //convert stereo to mono
        audio = malloc(samplecount*sizeof(short)/2);
        for(int k=0;k<samplecount/2;k++) {
            audio[k] = ((int)samples[k*2]+(int)samples[k*2+1])/2; //prevent overflow
        }
        free(samples);
    }

    alGenBuffers(1,&wav->openal_buffer);
    alBufferData(wav->openal_buffer,AL_FORMAT_MONO16,(channels>1)?audio:samples,samplecount*sizeof(short)/channels,samplerate);

    wav->min = min;
    wav->max = max;
}

void sound_init() {
    ALCdevice* device = alcOpenDevice(NULL);

    if(!device) {
        sound_enabled = 0;
        printf("Could not open sound device!\n");
        return;
    }

    ALCcontext* context = alcCreateContext(device,NULL);
    if(!alcMakeContextCurrent(context)) {
        sound_enabled = 0;
        printf("Could not enter sound device context!\n");
        return;
    }

    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

    sound_load(&sound_footstep1,"wav/footstep1.wav",0.1F,32.0F);
    sound_load(&sound_footstep2,"wav/footstep2.wav",0.1F,32.0F);
    sound_load(&sound_footstep3,"wav/footstep3.wav",0.1F,32.0F);
    sound_load(&sound_footstep4,"wav/footstep4.wav",0.1F,32.0F);

    sound_load(&sound_wade1,"wav/wade1.wav",0.1F,32.0F);
    sound_load(&sound_wade2,"wav/wade2.wav",0.1F,32.0F);
    sound_load(&sound_wade3,"wav/wade3.wav",0.1F,32.0F);
    sound_load(&sound_wade4,"wav/wade4.wav",0.1F,32.0F);

    sound_load(&sound_jump,"wav/jump.wav",0.1F,32.0F);
    sound_load(&sound_land,"wav/land.wav",0.1F,32.0F);
    sound_load(&sound_jump_water,"wav/waterjump.wav",0.1F,32.0F);
    sound_load(&sound_land_water,"wav/waterland.wav",0.1F,32.0F);

    sound_load(&sound_explode,"wav/explode.wav",0.1F,53.0F);
    sound_load(&sound_explode_water,"wav/waterexplode.wav",0.1F,53.0F);
    sound_load(&sound_grenade_bounce,"wav/grenadebounce.wav",0.1F,48.0F);
    sound_load(&sound_grenade_pin,"wav/pin.wav",0.1F,48.0F);

    sound_load(&sound_hurt_fall,"wav/fallhurt.wav",0.1F,32.0F);

    sound_load(&sound_pickup,"wav/pickup.wav",0.1F,1024.0F);
    sound_load(&sound_horn,"wav/horn.wav",0.1F,1024.0F);

    sound_load(&sound_rifle_shoot,"wav/semishoot.wav",0.1F,48.0F);
    sound_load(&sound_rifle_reload,"wav/semireload.wav",0.1F,16.0F);
    sound_load(&sound_smg_shoot,"wav/smgshoot.wav",0.1F,48.0F);
    sound_load(&sound_smg_reload,"wav/smgreload.wav",0.1F,16.0F);
    sound_load(&sound_shotgun_shoot,"wav/shotgunshoot.wav",0.1F,48.0F);
    sound_load(&sound_shotgun_reload,"wav/shotgunreload.wav",0.1F,16.0F);
    sound_load(&sound_shotgun_cock,"wav/cock.wav",0.1F,16.0F);

    sound_load(&sound_hitground,"wav/hitground.wav",0.1F,32.0F);
    sound_load(&sound_hitplayer,"wav/hitplayer.wav",0.1F,32.0F);
    sound_load(&sound_build,"wav/build.wav",0.1F,32.0F);

    sound_load(&sound_spade_woosh,"wav/woosh.wav",0.1F,32.0F);
    sound_load(&sound_spade_whack,"wav/whack.wav",0.1F,32.0F);

    sound_load(&sound_death,"wav/death.wav",0.1F,24.0F);
    sound_load(&sound_beep1,"wav/beep1.wav",0.1F,1024.0F);
    sound_load(&sound_beep2,"wav/beep2.wav",0.1F,1024.0F);
    sound_load(&sound_beep2,"wav/beep2.wav",0.1F,1024.0F);
    sound_load(&sound_switch,"wav/switch.wav",0.1F,1024.0F);
    sound_load(&sound_empty,"wav/empty.wav",0.1F,1024.0F);
    sound_load(&sound_intro,"wav/intro.wav",0.1F,1024.0F);

    sound_load(&sound_debris,"wav/debris.wav",0.1F,53.0F);
    sound_load(&sound_bounce,"wav/bounce.wav",0.1F,32.0F);
    sound_load(&sound_impact,"wav/impact.wav",0.1F,53.0F);

    memset(sound_sources_free,1,sizeof(sound_sources_free));
}