package csv

import (
	"encoding/csv"
	"os"
	"reflect"
	"fmt"
	"strconv"
)


/*
暂时只支持int,bool,float,string 注：bool（0和非0）
*/
type CsvReader struct {
}


func (cr *CsvReader) ReadStruct(filePath string, comma, comment rune, p reflect.Type) ([]interface{}, error) {
	if p.Kind() == reflect.Ptr {
		p = p.Elem()
	}
	fieldNum := p.NumField()

	check_v := reflect.New(p)
	for i := 0;i < fieldNum;i++ {
		tField := p.Field(i)
		if !check_v.Elem().Field(i).CanSet() {
			return nil, fmt.Errorf("cfg:[%s] field:[%s] not export", p.Name(), tField.Name)
		}

		kind := tField.Type.Kind()
		if kind != reflect.Bool &&
			kind != reflect.Int && kind != reflect.Int8 && kind != reflect.Int16 && kind != reflect.Int32 && kind != reflect.Int64 &&
		    kind != reflect.Uint && kind != reflect.Uint8 && kind != reflect.Uint16 && kind != reflect.Uint32 && kind != reflect.Uint64 &&
		    kind != reflect.Float32 && kind !=  reflect.Float64  &&
		    kind != reflect.String	{

			sts_func_str := tField.Tag.Get("csvsts")
			if len(sts_func_str) == 0 {
				return nil, fmt.Errorf("cfg:[%s] field:[%s] Type:[%s] not support in csv",  p.Name(), tField.Name, tField.Type.Kind())
			}

			sts_func := check_v.MethodByName(sts_func_str)
			if !sts_func.IsValid() {
				err := fmt.Errorf("cfg:[%s] not found func:[%s]", p.Name(), sts_func_str)
				return nil, err
			}

			if sts_func.Type().NumIn() != 1 || sts_func.Type().In(0) != reflect.TypeOf("") {
				err := fmt.Errorf("cfg:[%s] func:[%s] params cnt must be 1 and type is string", p.Name(), sts_func_str)
				return nil, err
			}

			if sts_func.Type().NumOut() != 1 || sts_func.Type().Out(0) != reflect.TypeOf((*error)(nil)).Elem() {
				err := fmt.Errorf("cfg:[%s] func:[%s] returns cnt must be 1 and type is error", p.Name(), sts_func_str)
				return nil, err
			}
		}
	}

	file, err := os.Open(filePath)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	reader := csv.NewReader(file)
	reader.Comma = comma
	reader.Comment = comment
	lines, err := reader.ReadAll()
	if err != nil {
		return nil, err
	}

	if len(lines) < 3 {
		return nil, fmt.Errorf("file:[%s] first,second,third line must be names,descs,types", filePath)
	}

	if fieldNum != len(lines[0]) ||  fieldNum != len(lines[2]) {
		return nil, fmt.Errorf("file:[%s] fieldNum:%d names and types field num not match", filePath, fieldNum)
	}

	results := make([]interface{}, len(lines) - 3)
	for n := 3; n < len(lines); n++ {
		line := lines[n]
		if len(line) != fieldNum {
			return nil, fmt.Errorf("file:[%s] fieldNum:%d line:%d field num not match", filePath, fieldNum, n+1)
		}

		cfg := reflect.New(p).Interface()
		tv := reflect.ValueOf(cfg)
		for i := 0; i < fieldNum;i++ {
			strField := line[i]
			field := tv.Elem().Field(i)
			kind := field.Kind()
			switch(kind) {
			case reflect.Bool:
				var v bool
				v, err = strconv.ParseBool(strField)
				if err == nil {
					field.SetBool(v)
				}
			case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
				var v int64
				v, err = strconv.ParseInt(strField, 0, field.Type().Bits())
				if err == nil {
					field.SetInt(v)
				}
			case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
				var v uint64
				v, err = strconv.ParseUint(strField, 0, field.Type().Bits())
				if err == nil {
					field.SetUint(v)
				}
			case reflect.Float32, reflect.Float64:
				var v float64
				v, err = strconv.ParseFloat(strField,  field.Type().Bits())
				if err == nil {
					field.SetFloat(v)
				}
			case reflect.String:
				field.SetString(strField)
			default:
				sts_func := tv.MethodByName(tv.Elem().Type().Field(i).Tag.Get("csvsts"))
				rets := sts_func.Call([]reflect.Value{reflect.ValueOf(strField)})
				if rets[0].Interface() != nil {
					err =  rets[0].Interface().(error)
				}
			}

			if err != nil {
				return nil, fmt.Errorf("file:[%s] line:%d field:%s err:%v", filePath, n+1, tv.Elem().Type().Field(i).Name, err)
			}
		}

		results[n-3] = cfg
	}

	return results, nil
}