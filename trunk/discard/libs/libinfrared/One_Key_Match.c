/***********************************************************************************
 * 文 件 名   : One_Key_Match.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年7月19日
 * 文件描述   : 红外转发器一键匹配功能
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

#include <stdlib.h>
#include "One_key_match.h"
#include "comParse.h"
#include "DeviceCode.h"
#include "logUtils.h"

#define MAX_CODELIB_REPORT	20

uint8_t IRC_Remote_Learn_Count = 0;
//定义接设备类型
uint8_t IRC_Remote_Learn_Device_Type = 0;
//定义想获取数据类型
uint8_t IRC_Remote_Learn_Device_Data_Type = 0;

//定义设备品牌个数
typedef enum{
	ARC_DEVICE_NUM = 67,
}device_num_t;
//定义空调设备每个品牌 的设备个数


typedef enum{
	Midea_Brand = 63,
	Toshiba_Brand = 30,
	Haier_Brand = 45,
	Gree_Brand = 60,
	Chigo_Brand =45 ,
	Oaks_Brand = 45,
	Chunlan_Brand = 30,
	LG_Brand =20 ,
	TCL_Brand = 20,
	Samsung_Brand = 40,
	Small_Samsung_Brand = 30,
	Sanyo_Brand = 90,
	MITSUBISHI_heavy_Brand = 60,
	MITSUBISHI_motor_Brand =100 ,
	Changhong_Brand = 35,
	Panasonic_Brand =35 ,
	Sound_of_music_Brand =30 ,
	Uchida_Brand =20 ,
	Hisense_Brand =20 ,
	Shinco_Brand =20 ,
	Hitachi_Brand =35 ,
	Hualing_Brand = 24,
	Kelon_Brand =50 ,
	Warburg_Brand = 40,
	Might_Brand =15 ,
	Changling_Brand =26 ,
	Ying_Yan_Brand =16,
	Blue_Wave_Brand =16,
	Rabbit_Brand =16,
	Carrier_Brand =20,
	Toho_Brand =10,
	Gaoluhua_Brand =22,
	Aucma_Brand =18,
	Shengfeng_Brand = 16,
	Yangzi_Brand =10,
	Whirlpool_Brand =10 ,
	Daikin_Brand =30 ,
	HSBC_Brand =10 ,
	Toshiba2_Brand =45,
	Electrolux_Brand =15 ,
	Panda_Brand =10,
	Ling_on_Brand =10 ,
	Tianyuan_Brand =21,
	Other_brands  =468,
	Lily_Brand =100,
	SimpLlcrty_Brand =100,
	Daewoo_Brand =100,
	Clarent_Brand =100,
	Colones_Brand =100,
	Southwind_Brand =100,
	Kunda_Brand =100,
	Yushchenko_Brand =100,
	Freenet_Brand =100,
	Shengfeng2_Brand =100,
	Thailand_Brand =100,
	Glanz_Brand= 50,
	Leroy_Brand= 30,
	Fujitsu_Brand=  30,
	York_Brand= 27,
	McQuay_Brand =27,
	sanken_Brand= 32,
	polytron_Brand= 30,
	Sharp_Brand= 30,
	ZHEN_BAO_Brand =11,
	WANBAO_Brand =14,
	YINGHUA_Brand= 17
}arc_device_Type_Num_t;


const uint16_t Brand[66] = {
	Midea_Brand,
	Toshiba_Brand,
	Haier_Brand,
	Gree_Brand,
	Chigo_Brand ,
	Oaks_Brand,
	Chunlan_Brand,
	LG_Brand,
	TCL_Brand,
	Samsung_Brand ,
	Small_Samsung_Brand ,
	Sanyo_Brand ,
	MITSUBISHI_heavy_Brand ,
	MITSUBISHI_motor_Brand  ,
	Changhong_Brand ,
	Panasonic_Brand ,
	Sound_of_music_Brand ,
	Uchida_Brand ,
	Hisense_Brand ,
	Shinco_Brand ,
	Hitachi_Brand  ,
	Hualing_Brand ,
	Kelon_Brand ,
	Warburg_Brand ,
	Might_Brand ,
	Changling_Brand ,
	Ying_Yan_Brand ,
	Blue_Wave_Brand,
	Rabbit_Brand ,
	Carrier_Brand ,
	Toho_Brand ,
	Gaoluhua_Brand,
	Aucma_Brand ,
	Shengfeng_Brand ,
	Yangzi_Brand ,
	Whirlpool_Brand ,
	Daikin_Brand ,
	HSBC_Brand  ,
	Toshiba2_Brand ,
	Electrolux_Brand ,
	Panda_Brand ,
	Ling_on_Brand ,
	Tianyuan_Brand ,
	Other_brands ,
	Lily_Brand,
	SimpLlcrty_Brand,
	Daewoo_Brand ,
	Clarent_Brand,
	Colones_Brand ,
	Southwind_Brand ,
	Kunda_Brand ,
	Yushchenko_Brand ,
	Freenet_Brand ,
	Shengfeng2_Brand ,
	Thailand_Brand ,
	Glanz_Brand,
	Leroy_Brand,
	Fujitsu_Brand,
	York_Brand,
	McQuay_Brand,
	sanken_Brand,
	polytron_Brand,
	Sharp_Brand,
	ZHEN_BAO_Brand,
	WANBAO_Brand,
	YINGHUA_Brand
};


#define SORTMAXSIZE 512
sortOneKeyMatch_t SortList[SORTMAXSIZE] = {{0,0}};

int OneKeyMatch(hostCmd *dst,char *libsrc);
int StartOneKeyMatch(oneKeyMatchInfo_t *brandInfo,int brandSize,hostCmd *srcCmd,hostCmd *dstCmd);
int GetLibIndex(uint16_t brandIndex,uint16_t deviceType);


//一键匹配统一接口
int OneKeyMatchDevice(hostCmd *srcCmd,hostCmd *dstCmd)
{
	oneKeyMatchInfo_t *brandInfo = NULL;
	int brandSize = 0;
	switch(IRC_Remote_Learn_Device_Type)
	{
		case IRC_DEVICE_TYPE_PJT:
		#if PJT_ENABLE
		
		#endif
		break;
	case IRC_DEVICE_TYPE_FAN:
		#if FAN_ENABLE
		
		#endif
		break;
	case IRC_DEVICE_TYPE_STB:
		#if STB_ENABLE
			brandInfo = (oneKeyMatchInfo_t*)stb_one_key_match_info_struct;
			brandSize = STB_BRAND_COUNT;
		#endif
		break;
	case IRC_DEVICE_TYPE_DVD:
	#if DVD_ENABLE
		
		#endif
		break;
	case IRC_DEVICE_TYPE_TV:
		#if TV_ENABLE
			#if ONE_KEY_MATCH_TV_ENABLE
			brandInfo = (oneKeyMatchInfo_t*)tv_one_key_match_info_struct;
			brandSize = TV_BRAND_COUNT;
			#endif
		#endif
		break;
	case IRC_DEVICE_TYPE_IPTV:
		#if IPTV_ENBALE
			
		#endif
		break;
	case IRC_DEVICE_TYPE_ARC:
		#if ARC_ENABLE
			#if ONE_KEY_MATCH_ARC_ENABLE
			brandInfo = (oneKeyMatchInfo_t*)arc_one_key_match_info_struct;
			brandSize = ARC_BRAND_COUNT;
			#endif
		#endif
		break;
	default:
		break;
	}
	if((brandInfo!=NULL)&&(brandSize!=0))
		return StartOneKeyMatch(brandInfo,brandSize,srcCmd,dstCmd);
	return -1;
}

int SortCompareMatching(const void *a,const void *b)
{
	sortOneKeyMatch_t *src = (sortOneKeyMatch_t*)a;
	sortOneKeyMatch_t *dst = (sortOneKeyMatch_t*)b;
	if(src->Matching > dst->Matching)
		return -1;
	if(src->Matching == dst->Matching)
		return 0;
	if(src->Matching < dst->Matching)
		return 1;
}

//删除重复数据，必须先排序
int RemoveDuplates(sortOneKeyMatch_t A[], uint16_t nCnt)
{
    uint16_t nNewLen = nCnt;
    uint16_t j       = 0;
	uint16_t i	   = 0;

//	log_debug("nNewLen = %d\n",nNewLen);
	for(i=0;i<nCnt;i++)
	{
		if(A[i].ArcLibIndex == 0 || A[i].Matching == 0)
			continue;
		
		for(j=i+1;j<nCnt;j++)
		{
			if(A[i].ArcLibIndex == A[j].ArcLibIndex)
			{
				//log_debug(" [%d : %d] [%d : %d]\n",i,j,A[i].ArcLibIndex,A[j].ArcLibIndex);
				A[j].ArcLibIndex = 0;
				A[j].Matching = 0;
				nNewLen--;
			}
		}
	}
//	log_debug("nNewLen = %d\n",nNewLen);
    return nNewLen;
}

int StartOneKeyMatch(oneKeyMatchInfo_t *brandInfo,int brandSize,hostCmd *srcCmd,hostCmd *dstCmd)
{
	uint16_t MaxSimilar = 0;
	uint16_t count = 0;
	int Similar = 0;
	char *pchar = NULL;
	uint16_t BrandCount = 0;
	uint16_t BrandTypeCount = 0;
	uint16_t sortListIndex = 0;
	for (BrandCount = 0;BrandCount<brandSize;BrandCount++)
	{
		for(BrandTypeCount = 0;BrandTypeCount<brandInfo[BrandCount].LibSize;BrandTypeCount++)
		{
			//pchar = (char*)arc_one_key_match_info[BrandCount][BrandTypeCount];
			pchar = (char*)brandInfo[BrandCount].oneKeyMatchLib[BrandTypeCount];
			Similar = OneKeyMatch(srcCmd,pchar);
			
			//将结果放到的队列里面
			if(Similar >= 80)
			{
				if(sortListIndex < SORTMAXSIZE)
				{
					SortList[sortListIndex].Matching = Similar;
					SortList[sortListIndex].ArcLibIndex = GetDeviceIndexInTable(IRC_Remote_Learn_Device_Type,BrandCount,BrandTypeCount+1);
					sortListIndex++;
				}
				else
				{
					qsort(SortList,SORTMAXSIZE,sizeof(SortList[0]),SortCompareMatching);
					if(Similar > SortList[SORTMAXSIZE-1].Matching)
					{
						SortList[SORTMAXSIZE-1].Matching = Similar;
						SortList[SORTMAXSIZE-1].ArcLibIndex = GetDeviceIndexInTable(IRC_Remote_Learn_Device_Type,BrandCount,BrandTypeCount+1);
					}
				}
			}
		}
	}
	
	log_debug("sortListIndex = %d\n",sortListIndex);

	//依据相识度排序
	qsort(SortList,sortListIndex,sizeof(SortList[0]),SortCompareMatching);

	MaxSimilar = RemoveDuplates(SortList,sortListIndex);

	//log_debug("MaxSimilar1 = %d\n",MaxSimilar);
	//依据相识度排序
	qsort(SortList,sortListIndex,sizeof(SortList[0]),SortCompareMatching);

	MaxSimilar = MaxSimilar>=MAX_CODELIB_REPORT?MAX_CODELIB_REPORT:MaxSimilar;

	dstCmd->idx = 0;

	log_debug("MaxSimilar2 = %d\n",MaxSimilar);
	
	for(count=0;count<MaxSimilar;count++)
	{
//		log_debug("ArcLibIndex = %d\n",SortList[count].ArcLibIndex);
		cmdSet16bitVal(dstCmd,SortList[count].ArcLibIndex);
	}

	log_debug("StartOneKeyMatch--\n");
	return MaxSimilar;
}

//src :匹配库中的值，dst:学习到的值
//返回匹配的字节数
int OneKeyMatch(hostCmd *dst,char *libsrc)
{
	uint8_t dstObject= 0;
	uint16_t SameCount = 0;
	int srcCurrentData = 0;
	uint8_t Count = 0;
	//数据从第0个开始比较
	dst->idx = 0;
	//2个字符表示一个字节所以除以2
	uint16_t srcLength = strlen(libsrc)>>1;
	uint16_t length = dst->size >= srcLength?srcLength:dst->size;
	
	for(Count=0;Count < length;Count++)
	{
		//将字符串转为16进制数据
		srcCurrentData = StrToHex(libsrc,Count);
		
		if(-1 != srcCurrentData)
		{
			cmdGet8bitVal(dst,&dstObject);
			//单个字节小于15内算相同
			if((abs(srcCurrentData - dstObject)) < 15)
			{
				SameCount++;
			}
		}
	}
	
	//重新统计
	dst->idx = 0;
	return ((double)SameCount / length) * 100;
}

int StrToHex(char *p,uint8_t index)
{
	char data[] = {0,0};
	uint16_t indexs = index<<1 ;
	if(strlen(p) > indexs)	
	{
		data[0] = p[indexs];
		data[1] = p[indexs+1];
		return (int)strtol(data,NULL,16);
	}
	return -1;
}

