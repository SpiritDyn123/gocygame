package config

import (
	"github.com/SpiritDyn123/gocygame/libs/config/csv"
	"reflect"
)

type IConfigScriptSink interface {
	GetFilePath() string
	OnConfigLoad([]interface{}) error
	OnConfigUpdate([]interface{}) error
	GetCfgType() reflect.Type
}

type IConfigScriptManager interface {
	LoadConfig(sink IConfigScriptSink) error
	UpdateConfig(sink IConfigScriptSink) error
}

type configCsvScriptManager struct {
	comma rune
	comment rune
}

func (csm *configCsvScriptManager) LoadConfig(sink IConfigScriptSink) (err error) {
	cfgs, err := (&csv.CsvReader{}).ReadStruct(sink.GetFilePath(), csm.comma, csm.comment, sink.GetCfgType())
	if err != nil {
		return err
	}

	return sink.OnConfigLoad(cfgs)
}

func (csm *configCsvScriptManager) UpdateConfig(sink IConfigScriptSink) (err error) {
	cfgs, err := (&csv.CsvReader{}).ReadStruct(sink.GetFilePath(), csm.comma, csm.comment, sink.GetCfgType())
	if err != nil {
		return err
	}

	return sink.OnConfigUpdate(cfgs)
}

func NewCsvConfigScriptManager(comma, comment rune) IConfigScriptManager {
	csm := &configCsvScriptManager{
		comma:comma,
		comment:comment,
	}

	return csm
}