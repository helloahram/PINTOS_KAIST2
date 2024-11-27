/* vm.c: Generic interface for virtual memory objects. */

#include "vm/vm.h"
#include "threads/malloc.h"
#include "vm/inspect.h"
#include <vaddr.h>

/* Project3 frame_table, frame_start */
static struct list frame_table;
static struct list_elem *frame_start;

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void vm_init(void) {
    vm_anon_init();
    vm_file_init();
#ifdef EFILESYS /* For project 4 */
    pagecache_init();
#endif
    register_inspect_intr();
    /* DO NOT MODIFY UPPER LINES. */
    /* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type page_get_type(struct page *page) {
    int ty = VM_TYPE(page->operations->type);
    switch (ty) {
    case VM_UNINIT:
        return VM_TYPE(page->uninit.type);
    default:
        return ty;
    }
}

/* Helpers */
static struct frame *vm_get_victim(void);
static bool vm_do_claim_page(struct page *page);
static struct frame *vm_evict_frame(void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage, bool writable,
                                    vm_initializer *init, void *aux) {

    ASSERT(VM_TYPE(type) != VM_UNINIT)

    struct supplemental_page_table *spt = &thread_current()->spt;

    /* Check wheter the upage is already occupied or not. */
    if (spt_find_page(spt, upage) == NULL) {
        /* TODO: Create the page, fetch the initialier according to the VM type,
         * TODO: and then create "uninit" page struct by calling uninit_new. You
         * TODO: should modify the field after calling the uninit_new. */

        /* TODO: Insert the page into the spt. */
    }
err:
    return false;
}

/* Project3 - Find VA from spt and return page. On error, return NULL. */
struct page *spt_find_page(struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
    /* TODO: Fill this function. */

    /* hash_find 호출을 위해 임시 페이지 구조체 생성 */
    struct page *page = (struct page *)malloc(sizeof(struct page));
    /* va 를 페이지 경계 주소로 맞추고, 페이지의 va 필드에 저장 */
    page->va = pg_round_down(va);
    /* spt Table 에서 해당 hash_elem 을 찾아 e 에 저장 */
    struct hash_elem *e = hash_find(&spt->spt_hash, &page->hash_elem);

    free(page); /* 임시로 생성한 페이지 객체 삭제 */

    if (e != NULL) { /* va 에 해당되는 hash_elem 이 있으면 페이지 반환 */
        return hash_entry(e, struct page, hash_elem);
    }
    /* va 에 해당 되는 페이지가 없다면 NULL 반환 */
    return NULL;
}

/* Project3 - Insert PAGE into spt with validation. */
bool spt_insert_page(struct supplemental_page_table *spt UNUSED,
                     struct page *page UNUSED) {
    /* TODO: Fill this function. */

    /* hash_insert 에서 insert 가 성공하면 NULL, 실패하면 old 를 return */
    if (!hash_insert(&spt->spt_hash, &page->hash_elem))
        return true; /* Insert 성공하여 NULL return */

    /* 기존 값이 존재하여 삽입하지 못한 경우 old 를 Return 하여 !NULL */
    return false;
}

void spt_remove_page(struct supplemental_page_table *spt, struct page *page) {
    vm_dealloc_page(page);
    return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *vm_get_victim(void) {
    struct frame *victim = NULL;
    /* TODO: The policy for eviction is up to you. */

    return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *vm_evict_frame(void) {
    struct frame *victim UNUSED = vm_get_victim();
    /* TODO: swap out the victim and return the evicted frame. */

    return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *vm_get_frame(void) {
    /* TODO: Fill this function. */
    /* Project3 - Memory Management */
    struct frame *frame = (struct frame *)malloc(sizeof(struct frame));
    ASSERT(frame != NULL); /* Malloc 결과 확인 - frame 이 NULL 이면 시스템 중단 */

    frame->kva = palloc_get_page(PAL_USER); /* 실제 메모리에서 페이지 할당 */

    if (frame->kva == NULL) /* 페이지 할당 요청이 실패한다면 Swap Out 수행 */
        frame = vm_evict_frame();
    else /* 페이지 할당 요청이 성공한다면, Frame Table 에 추가 */
        list_push_back(&frame_table, &frame->frame_elem);

    frame->page = NULL;

    ASSERT(frame->page == NULL);
    return frame;
}

/* Growing the stack. */
static void vm_stack_growth(void *addr UNUSED) {}

/* Handle the fault on write_protected page */
static bool vm_handle_wp(struct page *page UNUSED) {}

/* Return true on success */
bool vm_try_handle_fault(struct intr_frame *f UNUSED, void *addr UNUSED, bool user UNUSED,
                         bool write UNUSED, bool not_present UNUSED) {
    struct supplemental_page_table *spt UNUSED = &thread_current()->spt;
    struct page *page = NULL;
    /* TODO: Validate the fault */
    /* TODO: Your code goes here */

    return vm_do_claim_page(page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void vm_dealloc_page(struct page *page) {
    destroy(page);
    free(page);
}

/* Claim the page that allocate on VA. */
bool vm_claim_page(void *va UNUSED) {
    struct page *page = NULL;
    /* TODO: Fill this function */

    return vm_do_claim_page(page);
}

/* Claim the PAGE and set up the mmu. */
static bool vm_do_claim_page(struct page *page) {
    struct frame *frame = vm_get_frame();

    /* Set links */
    frame->page = page;
    page->frame = frame;

    /* TODO: Insert page table entry to map page's VA to frame's PA. */

    return swap_in(page, frame->kva);
}

/* Initialize new supplemental page table */
void supplemental_page_table_init(struct supplemental_page_table *spt UNUSED) {
    hash_init(&spt->spt_hash, hash_func, less_func, NULL);
}

/* Copy supplemental page table from src to dst */
bool supplemental_page_table_copy(struct supplemental_page_table *dst UNUSED,
                                  struct supplemental_page_table *src UNUSED) {}

/* Free the resource hold by the supplemental page table */
void supplemental_page_table_kill(struct supplemental_page_table *spt UNUSED) {
    /* TODO: Destroy all the supplemental_page_table hold by thread and
     * TODO: writeback all the modified contents to the storage. */
}
