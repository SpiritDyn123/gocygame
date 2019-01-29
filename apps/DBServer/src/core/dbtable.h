/**
 * @file dbtable.h
 *
 * 分库分表的数据库操作逻辑
 */

#ifndef DIGIMON_DBSVR_DBTABLE_H_
#define DIGIMON_DBSVR_DBTABLE_H_

#include <cassert>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

#include "../proto/ErrCode.pb.h"
#include "sqlconnection.h"
#include "MysqlProxy.h"

/**
 * @brief 分库分表的数据库操作逻辑
 * @param DB_NUM 分库数量，不能超过100000
 * @param TBL_NUM 分表数量，不能超过100
 */
template <int DB_NUM, int TBL_NUM>
class DBTable {
public:
	/**
	 * @brief 构造函数
	 * @param uid hash_id
	 * @param db 数据库名字前缀。假设有数据库db_data_000，则前缀为db_data
	 * @param tbl 数据表名字前缀。假设有数据表t_data_0，则前缀为t_data
	 */
	DBTable(uint32_t uid, const char* db, const char* tbl)
		: m_uid(uid), m_db(db), m_tbl(tbl)
	{
		assert((DB_NUM > 0) && (DB_NUM < 100001) && (TBL_NUM > 0) && (TBL_NUM < 101));
		m_conn = MysqlProxy::Instance().GetSqlConById(db, get_dbid(m_uid));
		assert(m_conn);
		m_dbnum_width = get_num_width(DB_NUM);
		m_tblnum_width = get_num_width(TBL_NUM);
		m_sql << std::setprecision(3) << std::fixed;
	}

	virtual ~DBTable()
	{
	}

protected:
	mysqlpp::Query get_query(const char* str=0) { return m_conn->query(str); }
	mysqlpp::SimpleResult execute(const std::string& sql) { return m_conn->execute(sql); }
	mysqlpp::StoreQueryResult store(const std::string& sql) { return m_conn->store(sql); }
	SQLConnection& get_con(uint32_t uid) { return *m_conn; }

	mysqlpp::StoreQueryResult store2(const std::string& sql, int& retcode)
	{
		retcode = ErrCodeSucc;
		auto res = m_conn->store(sql);
		if (m_conn->errnum() > 0) {
			retcode = m_conn->errnum();
		}
		return res;
	}
	/**
	 * @brief 查找一条记录，并把得到的结果赋值给val。
	 * @param sql 执行sql必须得到有且仅有一条结果，如果没有结果，或者结果不唯一，则返回错误码
	 * @param retcode_on_inexist 记录不存在时返回的错误码
	 * @return 成功返回0，失败返回对应的错误码
	 */
	int select_one(const std::string& sql, uint32_t& val, ErrCodeType retcode_on_inexist = ErrCodeEntryNotFound)
	{
		mysqlpp::StoreQueryResult res = m_conn->store(sql);
		if (res && (res.num_rows() == 1)) {
			val = static_cast<uint32_t>(res[0][0]);
			return ErrCodeSucc;
		}

		if (!res) {
			return ErrCodeDB;
		}
		if (res.num_rows() == 0) {
			return retcode_on_inexist;
		}
		return ErrCodeMultiEntries;
	}
	/**
	 * @brief 查找一条记录，并把得到的结果赋值给vals。
	 * @param sql 执行sql必须得到有且仅有一条结果，如果没有结果，或者结果不唯一，则返回错误码
	 * @param retcode_on_inexist 记录不存在时返回的错误码
	 * @return 成功返回0，失败返回对应的错误码
	 */
	int select_one(const std::string& sql, uint32_t* vals, int val_len,
					ErrCodeType retcode_on_inexist = ErrCodeEntryNotFound)
	{
		mysqlpp::StoreQueryResult res = m_conn->store(sql);
		if (res && (res.num_rows() == 1)) {
			for (int i = 0; i < val_len; ++i) {
				vals[i] = static_cast<uint32_t>(res[0][i]);
			}
			return ErrCodeSucc;
		}

		if (!res) {
			return ErrCodeDB;
		}
		if (res.num_rows() == 0) {
			return retcode_on_inexist;
		}
		return ErrCodeMultiEntries;
	}
	/**
	 * @brief 查找记录，对于得到的每条记录，都会调用fn(row)。
	 * @return 成功返回0，失败返回1002
	 */
	template <typename Function>
	int select(const std::string& sql, Function& fn)
	{
		mysqlpp::UseQueryResult res = m_conn->use(sql);
		if (res) {
			while (mysqlpp::Row row = res.fetch_row()) {
				fn(row);
			}
			return ErrCodeSucc;
		}

		return ErrCodeDB;
	}
	/**
	 * @brief 更新一条记录
	 * @param sql 执行sql必须更新且仅更新一条记录，如果没有更新任何记录，或者更新了多条记录，则返回错误码
	 * @param retcode_on_inexist 记录不存在时返回的错误码
	 * @return 成功返回0，失败返回对应的错误码
	 */
	int update_one(const std::string& sql, ErrCodeType retcode_on_inexist = ErrCodeEntryNotFound)
	{
		mysqlpp::SimpleResult res = m_conn->execute(sql);
		if (res && (res.rows() == 1)) {
			return ErrCodeSucc;
		}

		if (!res) {
			return ErrCodeDB;
		}
		if (res.rows() == 0) {
			return retcode_on_inexist;
		}
		return ErrCodeMultiEntries;

	}
	/**
	 * @brief 插入一条记录
	 * @param retcode_on_dup 重复插入时返回的错误码
	 * @return 成功返回0，失败返回对应的错误码
	 */
	int insert(const std::string& sql, ErrCodeType retcode_on_dup = ErrCodeDupEntry)
	{
		mysqlpp::SimpleResult res = m_conn->execute(sql);
		if (res) {
			return ErrCodeSucc;
		}

		if (m_conn->errnum() == ER_DUP_ENTRY) {
			return retcode_on_dup;
		}
		return ErrCodeDB;
	}
	/**
	 * @brief 插入一条记录，并获取因此产生的自增ID
	 * @param retcode_on_dup 重复插入时返回的错误码
	 * @return 成功返回0，失败返回对应的错误码
	 */
	int insert(const std::string& sql, uint32_t& auto_id, ErrCodeType retcode_on_dup = ErrCodeDupEntry)
	{
		mysqlpp::SimpleResult res = m_conn->execute(sql);
		if (res) {
			auto_id = res.insert_id();
			return ErrCodeSucc;
		}

		if (m_conn->errnum() == ER_DUP_ENTRY) {
			return retcode_on_dup;
		}
		return ErrCodeDB;
	}
	/**
	 * @brief 对s进行转义
	 * @param s 需要被转义的字符串
	 * @return 转义后的字符串
	 */
	std::string escape_string(const std::string& s) const
	{
		std::string dst;
		m_conn->escape_string(dst, &s);
		return dst;
	}

	uint32_t get_dbid(uint32_t hash) const {
		return hash % DB_NUM;
	}

	/**
	 * @brief 获取正确的分表名称
	 * @param hash 根据hash进行哈希，获取正确的分表名称
	 * @return 返回正确的分表名称
	 */
	std::string get_dbname(uint32_t hash) const {
		static std::ostringstream oss;
		oss.str("");

		if (m_dbnum_width) {
			oss << m_db << '_' << std::setw(m_dbnum_width) << std::setfill('0') << get_dbid(hash);
		}
		else {
			oss << m_db;
		}
		return oss.str();
	}
	std::string get_table(uint32_t hash, bool withdb = true) const
	{
		static std::ostringstream oss;
		oss.str("");
		if (withdb) {
			if (m_dbnum_width) {
				oss << '`' << m_db << '_' << std::setw(m_dbnum_width) << std::setfill('0') << get_dbid(hash) << '`';
			}
			else {
				oss << '`' << m_db << '`';
			}
			// TODO: this->db->select_db(this->db_name); why?

			oss << '.';
		}
		if (m_tblnum_width) {
			oss << '`' << m_tbl << '_' << std::setw(m_tblnum_width) << std::setfill('0') << ((hash / DB_NUM) % TBL_NUM) << '`';
		} else {
			oss << '`' << m_tbl << '`';
		}

		return oss.str();
	}

private:
	// 返回分库分表的尾数位数
	int get_num_width(int n) const
	{
		if (n == 1) {
			return 0;
		} else if (n <= 10) {
			return 1;
		} else if (n <= 100) {
			return 2;
		} else if (n <= 1000) {
			return 3;
		} else if (n <= 10000) {
			return 4;
		} else if (n <= 100000) {
			return 5;
		} else {
			assert(!"invalid number: > 100000!!");
			return -1;
		}
	}

protected:
	/*! 用于构建SQL语句 */
	std::ostringstream m_sql;
	uint32_t	m_uid;

private:
	/*! 和数据库的连接对象 */
	SQLConnection* m_conn;
	std::string m_db;
	std::string m_tbl;
	// TODO:
	//std::string m_tables[DB_NUM * TBL_NUM];

	int m_dbnum_width;  // 分库位数，比如db_data_000，则该变量的值为3
	int m_tblnum_width; // 分表位数
};

/*
 class Ctable {
 protected:
 char dbser_return_buf[PROTO_MAX_SIZE];
 public:

 int select_data( char *&_p_result,	uint32_t &_pkg_len, MYSQL_RES* &res , uint32_t * p_count);

 int record_is_existed(char * sql_str,  bool * p_existed);
 int record_is_existed(char * sql_str,   int nofind_err);
 //影响行数为1行的 接口
 int exec_delete_sql(char * sql_str, int nofind_err );

 //影响行数为多行的接口( insert ,update ,delete,都可以使用)
 int exec_update_list_sql(char * sql_str, int nofind_err );

 Ctable(mysql_interface * db,const char * dbname,const char * tablename  );
 Ctable(mysql_interface * db);
 };




 class CtableRoute : public Ctable {
 protected:
 char id_name[20];
 char key2_name[20];
 public:
 //判断记录是否存在 通过 ID
 virtual int id_is_existed(uint32_t id, bool * existed);
 virtual int id_is_existed(uint32_t id, uint32_t key2, bool * existed);
 // table_name_pre : 表名的前面部分：如： t_user_pet_00 --  t_user_pet_99, 写入的是t_user_pet
 // id_name : 用于id_is_existed方法中
 CtableRoute(mysql_interface * db, const char * db_name_pre,
 const char * table_name_pre,const char* id_name ,const char* key2_name="") ;

 //设置int字段
 int set_int_value(userid_t userid ,const char * field_type , uint32_t  value);
 //得到int字段的值
 int get_int_value(userid_t userid ,const char * field_type ,  uint32_t * p_value);
 //设置int字段中的某一位, bitid  在[1..32]
 int set_int_value_bit(uint32_t userid ,
 const char * field_type ,uint32_t bitid ,uint32_t  value);

 //修改int字段的的值
 // changevalue : 修改多少
 // max_value 最大值
 // p_cur_value 返回当前值
 // p_real_change_value 返回实际修改多少
 // is_change_to_max_min  如果计算出的值大于最大值(或小于0)，是否设置为最大值(或0)
 int change_int_value(userid_t userid ,const char * field_type ,
 int32_t changevalue, uint32_t max_value , uint32_t *p_cur_value=NULL,
 int32_t *p_real_change_value=NULL, bool is_change_to_max_min=false);

 //使用userid ,和另一个key  为主键, key 如：装扮表中的 attireid (装扮ID )
 int set_int_value(userid_t userid ,uint32_t key2 ,const char * field_type , uint32_t  value);
 int get_int_value(userid_t userid,uint32_t key2 ,const char * field_type ,  uint32_t * p_value);
 int set_int_value_bit(uint32_t userid ,uint32_t key2,
 const char * field_type ,uint32_t bitid ,uint32_t  value);

 int change_int_value(userid_t userid ,uint32_t key2 ,const char * field_type ,
 int32_t changevalue, uint32_t max_value , uint32_t *p_cur_value=NULL,
 int32_t *p_real_change_value=NULL, bool is_change_to_max_min=false);

 int get_insert_sql_by_userid( userid_t userid, std::string & sql_str,
 const char * userid_field_name="userid" ,uint32_t obj_userid=0);
 };
 */

/*

int Ctable::exec_delete_sql(char * sql_str, int nofind_err )
{
	return this->exec_update_sql(sql_str,nofind_err  );
}

int  Ctable::record_is_existed(char * sql_str,  bool * p_existed)
{
	this->db->id=this->id;
	*p_existed=false;
	STD_QUERY_ONE_BEGIN(sql_str, SUCC);
		*p_existed=true;
	STD_QUERY_ONE_END();
}

int Ctable::exec_update_list_sql(char * sql_str, int nofind_err )
{
	int dbret;
	int acount;
	this->db->id=this->id;
	if ((dbret=this->db->exec_update_sql(sql_str,&acount ))==DB_SUCC)
	{
		return DB_SUCC;
	}else {
		return DB_ERR;
	}
}
int Ctable::select_data( char *&_p_result,	uint32_t &_pkg_len,
		MYSQL_RES* &res , uint32_t * p_count)
{

	this->db->id=this->id;

	if (( this->db->exec_query_sql(sqlstr,&res))!=DB_SUCC){
		return DB_ERR;
	}

    *p_count=mysql_num_rows(res);
	return SUCC;
}

int
Ctable::record_is_existed(char * sql_str,   int nofind_err)
{

	bool is_existed;
	int ret=this->record_is_existed( sql_str,  &is_existed );
	if (ret!=SUCC){
		return ret;
	}

	if  ( is_existed ){
		return SUCC;
	}else{
		return nofind_err;
	}
}




void str_addhex(std::string &to, char*from,int from_len  )
{
	char hex_buf[3];
	for (int i=0;i<from_len;i++){
		sprintf(hex_buf, "%02X",*((unsigned char *)&from[i]) );
		to+=hex_buf;
	}
}

CtableRoute::CtableRoute(mysql_interface * db,const  char * db_name_pre,
	const 	char * table_name_pre,const  char* id_name ,const char* key2_name )
	:Ctable(db,"","")
{
    strncpy(this->db_name_pre, db_name_pre,sizeof(this->db_name_pre));
	strncpy (this->table_name_pre,table_name_pre,sizeof(this->table_name_pre ) );
	strncpy (this->id_name,id_name,sizeof(this->id_name));
	strncpy (this->key2_name,key2_name,sizeof(this->key2_name));
}

int CtableRoute::id_is_existed(uint32_t id, uint32_t key2, bool * existed)
{

	sprintf( this->sqlstr, "select 1  from %s where %s=%u and %s=%u ",
		 this->get_table_name(id),this->id_name, id,this->key2_name,key2 );
		*existed=false;
	STD_QUERY_ONE_BEGIN(this->sqlstr, SUCC);
		*existed=true;
	STD_QUERY_ONE_END();
}


int CtableRoute::id_is_existed(uint32_t id, bool * existed)
{
	sprintf (this->sqlstr,"select  1 from %s where %s=%d ",
			this->get_table_name(id), this->id_name,id);
		*existed=false;
	STD_QUERY_ONE_BEGIN(this->sqlstr, SUCC);
		*existed=true;
	STD_QUERY_ONE_END();
}


int CtableRoute::get_int_value(userid_t userid ,const char * field_type ,  uint32_t * p_value)
{
	sprintf( this->sqlstr, "select  %s from %s where %s=%u ",
		field_type , this->get_table_name(userid),this->id_name, userid);
	STD_QUERY_ONE_BEGIN(this-> sqlstr,USER_ID_NOFIND_ERR);
		INT_CPY_NEXT_FIELD(*p_value );
	STD_QUERY_ONE_END();
}

int CtableRoute::set_int_value(userid_t userid ,const char * field_type , uint32_t  value)
{
	sprintf( this->sqlstr, "update %s set %s =%u where %s=%u " ,
		this->get_table_name(userid),field_type,value ,this->id_name, userid );
	return this->exec_update_sql(this->sqlstr,USER_ID_NOFIND_ERR );
}

int CtableRoute::set_int_value_bit(uint32_t userid ,const char * field_type ,uint32_t bitid ,uint32_t  value)
{
    if (bitid==0 ||  bitid>32 || value>1   ){
        return ENUM_OUT_OF_RANGE_ERR;
    }
    if (value==1){
        value=(1<<(bitid-1));
        sprintf( this->sqlstr, "update %s set %s =%s |%u    where %s=%u " ,
            this->get_table_name(userid),field_type,field_type ,  value , this->id_name ,userid );

    }else{
        value=0xFFFFFFFF-(1<<(bitid-1));
        sprintf( this->sqlstr, "update %s set %s =%s &%u    where %s=%u " ,
            this->get_table_name(userid),field_type,field_type ,  value , this->id_name ,userid );
    }
    return this->exec_update_sql(this->sqlstr,KEY_NOFIND_ERR );
}

int CtableRoute::change_int_value(userid_t userid ,const char * field_type ,
			   	int32_t changevalue, uint32_t max_value , uint32_t *p_cur_value,
				int32_t *p_real_change_value, bool is_change_to_max_min)
{
	uint32_t db_value;
	int ret;
	ret=this->get_int_value(userid,field_type,&db_value  );
	if(ret!=SUCC) return ret;
	int real_change_value=changevalue;
	int value= (int)db_value + changevalue;
	if (value<0){
		if (!is_change_to_max_min){
			return VALUE_NOENOUGH_E;
		}else{
			value=0;
			real_change_value=value-db_value;
		}
	} else if ((uint32_t) value> max_value ){
		if (!is_change_to_max_min){
			return VALUE_MAX_E;
		}else{
			value=max_value;
			real_change_value=value-db_value;
		}
	}
	if (p_cur_value!=NULL ){
		*p_cur_value=value;
	}
	if (p_real_change_value!=NULL ){
		*p_real_change_value=real_change_value;
	};
	if (real_change_value!=0){
		return this->set_int_value(userid,field_type,value );
	}else{
		return SUCC;
	}
}

int CtableRoute::change_int_value(userid_t userid ,uint32_t key2 ,const char * field_type ,
			   	int32_t changevalue, uint32_t max_value , uint32_t *p_cur_value,
				int32_t *p_real_change_value, bool is_change_to_max_min)
{
	uint32_t db_value;
	int ret;
	ret=this->get_int_value(userid,key2,field_type,&db_value  );
	if(ret!=SUCC) return ret;
	int real_change_value=changevalue;
	int value= (int)db_value + changevalue;
	DEBUG_LOG("db_value:%u, value:%d,max_value:%u",db_value,value,max_value);
	if (value<0){
		if (!is_change_to_max_min){
			return VALUE_NOENOUGH_E;
		}else{
			value=0;
			real_change_value=value-db_value;
		}
	} else if ((uint32_t) value> max_value ){
		if (!is_change_to_max_min){
			return VALUE_MAX_E;
		}else{
			value=max_value;
			real_change_value=value-db_value;
		}
	}
	if (p_cur_value!=NULL ){
		*p_cur_value=value;
	}

	if (p_real_change_value!=NULL ){
		*p_real_change_value=real_change_value;
	};
	if (real_change_value!=0){
		return this->set_int_value(userid,key2,field_type,value );
	}else{
		return SUCC;
	}
}

int CtableRoute::get_int_value(userid_t userid ,uint32_t key2 ,const char * field_type ,  uint32_t * p_value)
{
	sprintf( this->sqlstr, "select  %s from %s where %s=%u and %s=%u ",
		field_type , this->get_table_name(userid),this->id_name, userid,this->key2_name,key2 );
	STD_QUERY_ONE_BEGIN(this-> sqlstr,KEY_NOFIND_ERR);
		INT_CPY_NEXT_FIELD(*p_value );
	STD_QUERY_ONE_END();
}

int CtableRoute::set_int_value(userid_t userid ,uint32_t key2 ,const char * field_type , uint32_t  value)
{
	sprintf( this->sqlstr, "update %s set %s =%u where %s=%u and %s=%u " ,
		this->get_table_name(userid),field_type,value ,this->id_name, userid,this->key2_name,key2 );
	return this->exec_update_sql(this->sqlstr,KEY_NOFIND_ERR );
}

int CtableRoute::set_int_value_bit(uint32_t userid ,uint32_t key2 ,const char * field_type ,uint32_t bitid ,uint32_t  value)
{
    if (bitid==0 ||  bitid>32 || value>1   ){
        return ENUM_OUT_OF_RANGE_ERR;
    }
    if (value==1){
        value=(1<<(bitid-1));
        sprintf( this->sqlstr, "update %s set %s =%s |%u    where %s=%u and %s=%u  " ,
            this->get_table_name(userid),field_type,field_type ,  value ,
			this->id_name ,userid,this->key2_name,key2 );

    }else{
        value=0xFFFFFFFF-(1<<(bitid-1));
        sprintf( this->sqlstr, "update %s set %s =%s &%u    where %s=%u and %s=%u  " ,
            this->get_table_name(userid),field_type,field_type ,  value ,
			this->id_name ,userid,this->key2_name,key2 );
    }
    return this->exec_update_sql(this->sqlstr,USER_ID_NOFIND_ERR );
}

int CtableRoute::get_insert_sql_by_userid( userid_t userid, std::string & sql_str,const char * userid_field_name, uint32_t obj_userid )
{
	char buf[100000];
	if ( obj_userid==0){
		obj_userid=userid;
	}
	sprintf( this->sqlstr, "delete from  %s  where %s =%u  ;\n" ,
            this->get_table_name(obj_userid),userid_field_name,obj_userid );
	sql_str=this->sqlstr;

	sprintf( this->sqlstr, "select * from  %s  where %s =%u  " ,
            this->get_table_name(userid),userid_field_name,userid );
    MYSQL_RES *res;
	int ret;
	ret=this->db->exec_query_sql(this->sqlstr,  &res);
	if(ret!=SUCC) return ret;

	MYSQL_ROW row;
	MYSQL_FIELD   *field;
	if (mysql_num_rows(res)==0){
    	mysql_free_result(res);	//free result after you get the result
		return SUCC;
	};
	//得到字段类型
	mysql_field_seek(res,0);
	std::vector<int> is_num_field_list;
	int num_fields=mysql_num_fields(res);
	for (int i = 0; i <num_fields;  i++) {
		field= mysql_fetch_field(res);
		is_num_field_list.push_back (IS_NUM(field->type));
	}


	char obj_userid_str[100];
	sprintf(obj_userid_str ,"%u",obj_userid);
	sql_str+="insert into ";
	sql_str+= this->get_table_name(obj_userid);
	sql_str+= " values ";
    while ((row = mysql_fetch_row(res))!=NULL) {
		ulong *lengths= mysql_fetch_lengths(res);
		sql_str+="(";
		for (uint32_t i = 0; i < mysql_num_fields(res); i++) {
			if (row[i]==NULL){
				sql_str+="NULL" ;
			}else{
				if(i==0){//是userid
					sql_str+=obj_userid_str ;
				}else{
					if (is_num_field_list[i] ){
						sql_str+= row[i] ;
					}else{
						sql_str+= "'" ;
						set_mysql_string(buf,row[i],lengths[i]);
						sql_str+= buf ;
						sql_str+= "'" ;
					}
				}
			}
			sql_str+="," ;
		}
		sql_str.replace(sql_str.length()-1,1,1,')');
		sql_str+="," ;
    }
	sql_str.replace(sql_str.length()-1,1,1,';');
	sql_str+="\n" ;

    mysql_free_result(res);	//free result after you get the result
	return SUCC;
}






//依次得到row[i]
// 在STD_QUERY_WHILE_BEGIN  和 STD_QUERY_ONE_BEGIN
#define NEXT_FIELD 	 (row[++_fi])

#define PB_INT_CPY_NEXT_FIELD(v,member )  (v).set_##member(atoi_safe(NEXT_FIELD ));
#define PB_DOUBLE_CPY_NEXT_FIELD(v,member )  (v).set_##member(atof_safe(NEXT_FIELD ));
#define PB_BIN_CPY_NEXT_FIELD(v,member)  ++_fi; \
		mysql_fetch_lengths(res); \
		(v).set_##member(std::string(row[_fi],res->lengths[_fi]) );

#define PB_OBJ_CPY_NEXT_FIELD(v,member)  ++_fi; \
        mysql_fetch_lengths(res); \
        ((typeof(v).mutable_##member()  )((v).mutable_##member()))->ParseFromArray(row[_fi],res->lengths[_fi]);

#define BIN_CPY_NEXT_FIELD_TO_STRING(str )  ++_fi; \
		mysql_fetch_lengths(res); \
		(str).assign(row[_fi],res->lengths[_fi] );

//----------------------列表模式-----------------------------

#define STD_QUERY_WHILE_BEGIN( sqlstr,item_list )  \
	{ \
		typeof(item_list)&_item_list=item_list;\
		MYSQL_RES *res;\
		MYSQL_ROW  row;\
        this->db->id=this->id;\
		if (( this->db->exec_query_sql(sqlstr,&res))==DB_SUCC){\
			typeof(_item_list[0]) item ;\
			while((row = mysql_fetch_row(res))){\
				int _fi;\
			   	_fi=-1;


#define STD_QUERY_WHILE_END()  \
			_item_list.push_back(item);\
			}\
			mysql_free_result(res);	\
			return DB_SUCC;\
		}else {\
			return DB_ERR;\
		}\
	}

#define STD_QUERY_ONE_BEGIN( sqlstr, no_finderr ) {\
		uint32_t ret;\
		MYSQL_RES *res;\
		MYSQL_ROW row;\
		int rowcount;\
        this->db->id=this->id;\
		ret =this->db->exec_query_sql(sqlstr,&res);\
		if (ret==DB_SUCC){\
			rowcount=mysql_num_rows(res);\
			if (rowcount!=1) { \
	 			mysql_free_result(res);		 \
				DEBUG_LOG("no select a record [%u]",no_finderr );\
				return no_finderr;	 \
			}else { \
				row= mysql_fetch_row(res); \
				int _fi	 ; _fi=-1;



#define STD_QUERY_ONE_END()\
				mysql_free_result(res);\
				return DB_SUCC;\
			}\
		}else { \
			return DB_ERR;	 \
		}\
	}

#define STD_QUERY_ONE_END_WITHOUT_RETURN()  \
				mysql_free_result(res);		 \
			} \
		}else { \
			return DB_ERR;	 \
		}\
	}


#define PB_STD_QUERY_WHILE_BEGIN( sqlstr,p_item_list,member )  \
	{ \
		MYSQL_RES *res;\
		MYSQL_ROW  row;\
        this->db->id=this->id;\
		if (( this->db->exec_query_sql(sqlstr,&res))==DB_SUCC){\
			while((row = mysql_fetch_row(res))){\
			    typeof(*p_item_list->add_##member()) &item= *(p_item_list->add_##member());\
				int _fi;\
			   	_fi=-1;


#define PB_STD_QUERY_WHILE_END()  \
			}\
			mysql_free_result(res);	\
			return DB_SUCC;\
		}else {\
			return DB_ERR;\
		}\
	}

*/


#endif /* DIGIMON_DBSVR_DBTABLE_H_ */
