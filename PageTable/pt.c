#include "os.h"
#include <stdio.h>

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    /*We have 5 levels of the tree*/
    uint64_t mask = 0x1ff;
    uint64_t pa  = pt<<12; //Physical address for the frame

    for (int i = 0; i < 4; i++)
    {
        uint64_t curr_level_index = (vpn >> (9 * (4 - i))) & mask;

        uint64_t *va = (uint64_t *)phys_to_virt(pa);
        uint64_t pte = va[curr_level_index];
        if ((pte & 1) == 0)
        {
            if (ppn == NO_MAPPING)
            {
                return; // alrady no mapping
            }
            else
            {
                uint64_t new_pa_for_frame = (alloc_page_frame() << 12);
                uint64_t new_node_pte = new_pa_for_frame | 1; // returns a Page frame + adding 11 bits 0 and 1 as valid bit
                va[curr_level_index] = new_node_pte;
                pa = new_pa_for_frame;
            }
        }else{
            pa = pte - 1; //if pte valid it is pressented as pa
        }

    }

    uint64_t last_level_index = (vpn & mask);
    uint64_t *va = (uint64_t *)phys_to_virt(pa);
    if(ppn == NO_MAPPING){
        va[last_level_index] = 0;
    }
    else{
        va[last_level_index] = (ppn<<12)|1; //Updating in the last node (pt) the new mapping with eddtional 11 zeros and 1 valid bit

    }
}

uint64_t page_table_query(uint64_t pt , uint64_t vpn){
    uint64_t pa  = pt<<12;
    uint64_t *va = (uint64_t *)phys_to_virt(pa);
    uint64_t mask = 0x1ff;
    for (int i = 0; i < 4; i++)
    {
        uint64_t curr_level_index = (vpn >> (9 * (4 - i))) & mask;
        if(((va[curr_level_index])&1) == 1){//pte is valid          
            pa = va[curr_level_index] & 0xfffffffffffff000; 
            va = (uint64_t *)phys_to_virt(pa);
        }else{
            return NO_MAPPING;
        }
    }
    uint64_t last_level_index = (vpn & mask);
    if(((va[last_level_index])&1) == 1){
        return va[last_level_index]>>12;  // PTE : §§§§FRAME§§§0000001
    }else{
        return NO_MAPPING;
    }

}