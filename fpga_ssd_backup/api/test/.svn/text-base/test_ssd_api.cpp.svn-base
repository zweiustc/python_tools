/*
 *  Copyright (C) 2012,   baidu_ssd
 *  <wangyong03@baidu.com>, 2012.11s
 */
/**************************************
 * Header Include
 **************************************/
#include <../ssd_api.h>

#include <gtest/gtest.h>
#include <bmock.h>
using ::testing::Return;
using ::testing::_;
using ::testing::Invoke;

/**************************************
 * Globel variable and inline function
 **************************************/
//case number
 int test_case = 0;
//the file number of the dirpath
int gb_filecount = 0;

//function for testing
int ioctl_read_table_unittest(int fd, int type, void* );//struct ssd_table_param param);
int getMetadata(const char *dirpath, const struct stat *sb, int typeflag);

/**************************************
 *   start testing for ssd_api.c
 **************************************/
BMOCK_METHOD3(ioctl,int(int, int, void*));

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
 
/**
 * @brief 
**/
class test_SSD_open_suite : public ::testing::Test{
    protected:
        test_SSD_open_suite(){};
        virtual ~test_SSD_open_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_SSD_open_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_SSD_open_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_SSD_open_suite, case_name1)
{
      EXPECT_CALL(BMOCK_OBJECT(ioctl), ioctl(_, IOCTL_OPEN, _))
      .WillOnce(Return(1))
      .WillRepeatedly(Return(0));
 
      EXPECT_EQ(-1, SSD_open());
      EXPECT_EQ(0, SSD_open());
}
 
/**
 * @brief 
**/
class test_SSD_close_suite : public ::testing::Test{
    protected:
        test_SSD_close_suite(){};
        virtual ~test_SSD_close_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_SSD_close_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_SSD_close_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_SSD_close_suite, case_name1)
{
      EXPECT_CALL(BMOCK_OBJECT(ioctl), ioctl(_, IOCTL_CLOSE, _))
      .WillOnce(Return(1))
      .WillRepeatedly(Return(0));

      EXPECT_EQ(-1, SSD_close());
      SSD_open();
      EXPECT_EQ(0, SSD_close());
      SSD_open();
}
 
/**
 * @brief 
**/
class test_SSD_flush_suite : public ::testing::Test{
    protected:
        test_SSD_flush_suite(){};
        virtual ~test_SSD_flush_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_SSD_flush_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_SSD_flush_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_SSD_flush_suite, case_name1)
{
      EXPECT_CALL(BMOCK_OBJECT(ioctl), ioctl(_, IOCTL_FLUSH, _))
      .WillOnce(Return(1))
      .WillRepeatedly(Return(0));

      EXPECT_CALL(BMOCK_OBJECT(ioctl), ioctl(_, IOCTL_OPEN, _))
      .WillRepeatedly(Return(0));

      SSD_open();
      EXPECT_EQ(-1, SSD_flush());
      EXPECT_EQ(0, SSD_flush());
}
 
/**
 * @brief 
**/
class test_SSD_util_suite : public ::testing::Test{
    protected:
        test_SSD_util_suite(){};
        virtual ~test_SSD_util_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_SSD_util_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_SSD_util_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_SSD_util_suite, case_name1)
{
    EXPECT_CALL(BMOCK_OBJECT(ioctl), ioctl(_, IOCTL_READ_TABLE, _))
    .WillRepeatedly(Invoke(ioctl_read_table_unittest));

    test_case = 0;
    EXPECT_EQ(0ULL, SSD_util());

    test_case = 1;
    EXPECT_EQ(44 * 8000 * 2 * 1024 * 1024ULL, SSD_util());

    test_case = 2;
    EXPECT_EQ(22 * 8000 * 2 * 1024 * 1024ULL, SSD_util()); 
}

int ioctl_read_table_unittest(int fd, int type, void* table_param)
{
    int i;
    int ret = 0;

    struct ssd_table_param *param;
    memset(&param, 0, sizeof(struct ssd_table_param));
    param = (struct ssd_table_param*) table_param;

    switch(test_case)
    {
        case 0:
            ret = 1; break;
        case 1:
            for (i = 0; i < (SSD_MAX_VIRBLOCKS_PER_CHANNEL + PLANE_RESERVE_BLOCKS * 4) * 2; i+=2)
            {
                *(param->usr_buf + i) = 0x00;
                *(param->usr_buf + i + 1) = 0x00;
            }
            for (i = (SSD_MAX_VIRBLOCKS_PER_CHANNEL + PLANE_RESERVE_BLOCKS * 4) * 2; 
                   i < (SSD_MAX_PHYBLOCKS_PER_CHANNEL) * 2; i += 2)
            {
                *(param->usr_buf + i) = 0x00;
                *(param->usr_buf + i + 1) = 0x00;
            }
            break;
        case 2:
            for (i = 0; i < (SSD_MAX_VIRBLOCKS_PER_CHANNEL + PLANE_RESERVE_BLOCKS * 4) * 2; i+=2)
            {
                *(param->usr_buf + i) = 0x00;
                if (i % 4) 
                    *(param->usr_buf + i + 1) = 0x40;
                else
                    *(param->usr_buf + i + 1) = 0x00;
            }
            for (i = (SSD_MAX_VIRBLOCKS_PER_CHANNEL + PLANE_RESERVE_BLOCKS * 4) * 2;
                   i < (SSD_MAX_PHYBLOCKS_PER_CHANNEL) * 2; i += 2)
            {
                *(param->usr_buf + i) = 0x00;
                *(param->usr_buf + i + 1) = 0x00;
            }
            break;
        default:
            ret = 1;
            break;
    }

    return ret;
}


/**
 * @brief 
**/
class test_SSD_delete_suite : public ::testing::Test{
    protected:
        test_SSD_delete_suite(){};
        virtual ~test_SSD_delete_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_SSD_delete_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_SSD_delete_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_SSD_delete_suite, case_name1)
{
      EXPECT_CALL(BMOCK_OBJECT(ioctl),ioctl(_, IOCTL_FLUSH, _))
      .WillOnce(Return(1))
      .WillRepeatedly(Return(0));

      EXPECT_CALL(BMOCK_OBJECT(ioctl), ioctl(_, IOCTL_OPEN, _))
      .WillRepeatedly(Return(0));

      struct BlockId id;
      memset(&id, 0, sizeof(struct BlockId));
      
      SSD_open();
      EXPECT_EQ(-1, SSD_flush(id));
      EXPECT_EQ(0, SSD_flush(id));
}
 
/**
 * @brief 
**/
class test_SSD_write_suite : public ::testing::Test{
    protected:
        test_SSD_write_suite(){};
        virtual ~test_SSD_write_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_SSD_write_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_SSD_write_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_SSD_write_suite, case_name1)
{
      EXPECT_CALL(BMOCK_OBJECT(ioctl), ioctl(_, IOCTL_WRITE, _))
      .WillOnce(Return(0))
      .WillRepeatedly(Return(1<<7));

      EXPECT_CALL(BMOCK_OBJECT(ioctl), ioctl(_, IOCTL_WRITE_RETRY, _))
      .WillOnce(Return(1))
      .WillOnce(Return(1))
      .WillOnce(Return(1))
      .WillOnce(Return(1))
      .WillOnce(Return(1))
      .WillOnce(Return(1))
      .WillOnce(Return(1))
      .WillRepeatedly(Return(0));

      EXPECT_CALL(BMOCK_OBJECT(ioctl), ioctl(_, IOCTL_DELETE, _))
      .WillRepeatedly(Return(0));
   
      struct BlockId id;
      memset(&id, 0, sizeof(struct BlockId));

      char *buf;
      buf =(char *)malloc(BT_DATA_SIZE);
      uint32_t len = BT_DATA_SIZE;
      
      EXPECT_EQ(0, SSD_write(id, buf, len));
      EXPECT_EQ(-1, SSD_write(id, buf, len));
      EXPECT_EQ(0, SSD_write(id, buf, len));
      
      len = 2*2*1024*1024;
      EXPECT_EQ(-SSD_ERR_LEN_PAGE_ALIGN, SSD_write(id, buf, len));
}
 
/**
 * @brief 
**/
class test_SSD_read_suite : public ::testing::Test{
    protected:
        test_SSD_read_suite(){};
        virtual ~test_SSD_read_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_SSD_read_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_SSD_read_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_SSD_read_suite, case_name1)
{
      EXPECT_CALL(BMOCK_OBJECT(ioctl),ioctl(_, IOCTL_READ, _))
      .WillOnce(Return(1))
      .WillRepeatedly(Return(0));

      struct BlockId id;
      memset(&id, 0, sizeof(struct BlockId));

      char *buf;
      buf = (char *)malloc(BT_DATA_SIZE);

      uint32_t len = BT_DATA_SIZE;
      uint32_t offset = 0;

      EXPECT_EQ(-1, SSD_read(id, buf, offset,len));
      EXPECT_EQ(0, SSD_read(id, buf, offset, len));

      //param check
      len = 0;
      EXPECT_EQ(0, SSD_read(id, buf, offset, len));

      buf = NULL;
      EXPECT_EQ(-SSD_ERR_NULL, SSD_read(id, buf, offset, len));
      
      offset = 1;
      EXPECT_EQ(-SSD_ERR_START_PAGE_ALIGN, SSD_read(id, buf, offset, len));

      len = 1;
      EXPECT_EQ(-SSD_ERR_LEN_PAGE_ALIGN, SSD_read(id, buf, offset, len));

      len = BT_DATA_SIZE;
      offset = SSD_PAGE_SIZE;
      EXPECT_EQ(-SSD_ERR_PARAM_OVERFLOW, SSD_read(id, buf, offset, len)); 
}
 
/**
 * @brief 
**/
class test_SSD_ftw_suite : public ::testing::Test{
    protected:
        test_SSD_ftw_suite(){};
        virtual ~test_SSD_ftw_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_SSD_ftw_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_SSD_ftw_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_SSD_ftw_suite, case_name1)
{
    //set the behaviour of ioctl
    EXPECT_CALL(BMOCK_OBJECT(ioctl), ioctl(_, IOCTL_READ_ID_TABLE, _))
    .WillOnce(Return(-1))
    .WillOnce(Return(0))
    .WillOnce(Return(1))
    .WillOnce(Return(SSD_MAX_ID_NUM / 2))
    .WillOnce(Return(SSD_MAX_ID_NUM))
    .WillOnce(Return(SSD_MAX_ID_NUM + 1))
    .WillRepeatedly(Return(0));
  

    //start testing --------------------------------------------
    //case 0: ioctrl error    
    gb_filecount = 0;
    EXPECT_EQ(-1, SSD_ftw("/home", getMetadata, 100));
    EXPECT_EQ(0, gb_filecount);
    
    //case 1: empty id table
    gb_filecount = 0;
    EXPECT_EQ(0, SSD_ftw("/home", getMetadata, 100));
    EXPECT_EQ(0, gb_filecount);
 
    //case 2: only one valid
    gb_filecount = 0;
    EXPECT_EQ(0, SSD_ftw("/home", getMetadata, 100));
    EXPECT_EQ(1, gb_filecount);

    //case 3:half of the table
    gb_filecount = 0;
    EXPECT_EQ(0, SSD_ftw("/home", getMetadata, 100));
    EXPECT_EQ(SSD_MAX_ID_NUM / 2, gb_filecount);
    
    //case 4:full table
    gb_filecount = 0;
    EXPECT_EQ(0, SSD_ftw("/home", getMetadata, 100));
    EXPECT_EQ(SSD_MAX_ID_NUM, gb_filecount);
    
    //case 5:fatal return
    gb_filecount = 0;
    EXPECT_EQ(SSD_MAX_ID_NUM + 1, SSD_ftw("/home", getMetadata, 100));
    EXPECT_EQ(0, gb_filecount);


}


int getMetadata(const char *dirpath, const struct stat *sb, int typeflag)
{
    printf("file num:%d path:%s\n", ++gb_filecount, dirpath);

    printf("ID of device containing file:st_dev:%d\n",  sb->st_dev);
    printf("inode number:st_ino:%d\n",                  sb->st_ino);
    printf("protection? st_mode:%o   S_ISDIR:%d\n",     sb->st_mode, S_ISDIR(sb->st_mode));
    printf("number of hard links:st_nlink:%d\n",        sb->st_nlink);
    printf("st_uid:%d\n",                               sb->st_uid);
    printf("st_gid:%d\n",                               sb->st_gid);
    printf("device ID(if special file):st_rdev:%d\n",   sb->st_rdev);
    printf("st_size:%d\n",                              sb->st_size);
    printf("st_blksize(for filesystem io):%lu\n",       sb->st_blksize);
    printf("st_blocks(number of 512bytes blocks allocated):%lu\n", sb->st_blocks);
    printf("st_atime:%ld\n",                            sb->st_atime);
    printf("st_mtime:%ld\n",                            sb->st_mtime);
    printf("st_ctime:%ld\n",                            sb->st_ctime);

    struct tm *atime, *mtime, *ctime;    
    atime = localtime(&(sb->st_atime));
    mtime = localtime(&(sb->st_mtime));
    ctime = localtime(&(sb->st_ctime));

    printf("access time: %04d-%02d-%02d   %02d::%02d::%02d\n", 
            atime->tm_year+1900, atime->tm_mon+1, atime->tm_mday, 
            atime->tm_hour, atime->tm_min, atime->tm_sec);
    printf("modify time: %04d-%02d-%02d   %02d::%02d::%02d\n", 
            mtime->tm_year+1900, mtime->tm_mon+1, mtime->tm_mday, 
            mtime->tm_hour, mtime->tm_min, mtime->tm_sec);
    printf("change time: %04d-%02d-%02d   %02d::%02d::%02d\n", 
            ctime->tm_year+1900, ctime->tm_mon+1, ctime->tm_mday, 
            ctime->tm_hour, ctime->tm_min, ctime->tm_sec);

    printf("typeflag:%d\n", typeflag);
    printf("---------------------------------------------------------------\n");

    return 0;
}

 
