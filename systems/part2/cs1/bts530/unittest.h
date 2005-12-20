/* to compile:
* cxxtest/cxxtestgen.pl --error-printer -o unittest.cpp unittest.h
* c++ -I cxxtest unittest.cpp -o unittest
* */

#include <fcntl.h>
#include <cxxtest/TestSuite.h>

#include "bk.h"
#include "bkRead.h"

/* this is the class with the test functions,
* name is arbitrary,
* note what it inherits from
* */
class MyTestSuite : public CxxTest::TestSuite
{
    VolInfo volInfo;
    Dir tree;

    public:
    
    void oops(char* msg)
    {
        fflush(NULL);
        fprintf(stderr, "OOPS, %s\n", msg);
        exit(0);
    }

    void testStart(void)
    {
        int image;
        int rc;
        
        printf("\nTest Setup (opening slackd1-head.iso, reading RockRidge volume info)");
        
        /* open image file for reading */
        image = open("slackd1-head.iso", O_RDONLY);
        if(image == -1)
            oops("unable to open image");
        
        rc = readVolInfo(image, &volInfo);
        if(image <= 0)
            oops("failed to read volume info");
        
        tree.directories = NULL;
        tree.files = NULL;
        
        lseek(image, volInfo.pRootDrOffset, SEEK_SET);
        rc = readDir(image, &tree, FNTYPE_ROCKRIDGE, true);
        
        rc = close(image);
        if(rc == -1)
            oops("faled to close image");
    }
    
    void testRootDirs(void)
    {
        printf("\nTesting directories in root:\n");
        
        printf(" isolinux\n");
        TS_ASSERT( strcmp(tree.directories->dir.name, "isolinux") == 0 );
        
        printf(" kernels\n");
        TS_ASSERT( strcmp(tree.directories->next->dir.name, "kernels") == 0 );
        
        printf(" slackware\n");
        TS_ASSERT( strcmp(tree.directories->next->next->dir.name, "slackware") == 0 );
    }
    
    void testRootFiles(void)
    {
        printf("\nTesting files in root:\n");
        
        printf(" ANNOUNCE.10_1\n");
        TS_ASSERT( strcmp(tree.files->file.name, "ANNOUNCE.10_1") == 0 );
        
        printf(" BOOTING.TXT\n");
        TS_ASSERT( strcmp(tree.files->next->file.name, "BOOTING.TXT") == 0 );
        
        printf(" ChangeLog.txt\n");
        TS_ASSERT( strcmp(tree.files->next->next->file.name, "ChangeLog.txt") == 0 );
        
        printf(" CHECKSUMS.md5.asc\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->file.name, "CHECKSUMS.md5.asc") == 0 );
        
        printf(" CHECKSUMS.md5\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->file.name, "CHECKSUMS.md5") == 0 );
        
        printf(" COPYING\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->file.name, "COPYING") == 0 );
        
        printf(" COPYRIGHT.TXT\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->file.name, "COPYRIGHT.TXT") == 0 );
        
        printf(" CRYPTO_NOTICE.TXT\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->file.name, "CRYPTO_NOTICE.TXT") == 0 );
        
        printf(" FAQ.TXT\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->next->file.name, "FAQ.TXT") == 0 );
        
        printf(" FILELIST.TXT\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->next->next->file.name, "FILELIST.TXT") == 0 );
        
        printf(" GPG-KEY\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->next->next->next->file.name, "GPG-KEY") == 0 );
        
        printf(" PACKAGES.TXT\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->next->next->next->next->file.name, "PACKAGES.TXT") == 0 );
        
        printf(" README.TXT\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->next->next->next->next->next->file.name, "README.TXT") == 0 );
        
        printf(" RELEASE_NOTES\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->next->next->next->next->next->next->file.name, "RELEASE_NOTES") == 0 );
        
        printf(" Slackware-HOWTO\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->next->next->next->next->next->next->next->file.name, "Slackware-HOWTO") == 0 );
        
        printf(" SPEAKUP_DOCS.TXT\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->file.name, "SPEAKUP_DOCS.TXT") == 0 );
        
        printf(" SPEAK_INSTALL.TXT\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->file.name, "SPEAK_INSTALL.TXT") == 0 );
        
        printf(" UPGRADE.TXT\n");
        TS_ASSERT( strcmp(tree.files->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->file.name, "UPGRADE.TXT") == 0 );
    }

    void testEnd(void)
    {
        printf("\nTests Completed");
    }
};
