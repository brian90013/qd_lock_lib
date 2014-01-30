#ifndef READER_GROUPS_READ_INDICATOR_H
#define READER_GROUPS_READ_INDICATOR_H

/* Read Indicator */

#ifndef MRQD_LOCK_NUMBER_OF_READER_GROUPS
#    define MRQD_LOCK_NUMBER_OF_READER_GROUPS 64
#endif

#define MRQD_LOCK_READER_GROUP_PER_THREAD 1

typedef struct {
    LLPaddedUInt readerGroups[MRQD_LOCK_NUMBER_OF_READER_GROUPS];
} ReaderGroupsReadIndicator;

volatile atomic_int rgri_get_thread_id_counter = ATOMIC_VAR_INIT(0);
typedef union {
    int value;
    char pad[CACHE_LINE_SIZE];
} RGRIGetThreadIDVarWrapper;
_Alignas(CACHE_LINE_SIZE)
_Thread_local RGRIGetThreadIDVarWrapper rgri_get_thread_id_var = {.value = -1};

static inline
int rgri_get_thread_id(){
    //Warning this is not guranteed to work well on all platforms
    if(rgri_get_thread_id_var.value > -1) {
        return rgri_get_thread_id_var.value;
    } else {
        rgri_get_thread_id_var.value = atomic_fetch_add(&rgri_get_thread_id_counter, 1);
        return rgri_get_thread_id_var.value;
    }
}

void rgri_arrive(ReaderGroupsReadIndicator * indicator){
    int index = rgri_get_thread_id() % MRQD_LOCK_NUMBER_OF_READER_GROUPS;
#ifdef MRQD_LOCK_READER_GROUP_PER_THREAD
    atomic_store_explicit(&indicator->readerGroups[index].value, 1, memory_order_release);
#else
    atomic_fetch_add_explicit(&indicator->readerGroups[index].value, 1, memory_order_release);
#endif
}

void rgri_depart(ReaderGroupsReadIndicator * indicator){
    int index = rgri_get_thread_id() % MRQD_LOCK_NUMBER_OF_READER_GROUPS;
#ifdef MRQD_LOCK_READER_GROUP_PER_THREAD
    atomic_store_explicit(&indicator->readerGroups[index].value, 0, memory_order_release);
#else
    atomic_fetch_sub_explicit(&indicator->readerGroups[index].value, 1, memory_order_release);
#endif
}

void rgri_wait_all_readers_gone(ReaderGroupsReadIndicator * indicator){
    for(int i = 0; i < MRQD_LOCK_NUMBER_OF_READER_GROUPS; i++){
        atomic_thread_fence(memory_order_seq_cst);
        while(0 < atomic_load_explicit(&indicator->readerGroups[i].value, memory_order_relaxed)){
            thread_yield();
        }
    }
}

#endif
