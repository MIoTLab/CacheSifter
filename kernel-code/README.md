# CacheSifter: Sifting Cache Files for Boosted Mobile Performance and Lifetime

Mobile applications often maintain downloaded data as cache files in local storage for a better user experience. These cache files occupy a large portion of writes to mobile flash storage and have a significant impact on the performance and lifetime of mobile devices. Different from current practice, this paper proposes a novel framework, named CacheSifter, to differentiate cache files and treat cache files based on their reuse behaviors and main-memory/storage usages. Specifically, CacheSifter classifies cache files into three categories online and greatly reduces the number of writebacks on flash by dropping cache files that most likely will not be reused. We implement CacheSifter on real Android devices and evaluate it over representative applications. Paper Link: https://www.usenix.org/system/files/fast22-liang.pdf


The repository includes the source codes of CacheSifter for Huawei P9. 
