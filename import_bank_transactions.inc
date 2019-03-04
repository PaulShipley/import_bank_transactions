<?php
/**********************************************************************
// Creator: Paul Shipley
// date_:   2019-01-09
// Title:   Import Bank Transactions
// Free software under GNU GPL
***********************************************************************/

class import_bank_transaction
{
    private    $table = "IMPORT_BANK_TRANSACTIONS";

    function __construct()
    {

    }

    function init()
    {

    }

    function create_table()
    {
        $this->drop_table();

        $sql = "CREATE TABLE ".TB_PREF.$this->table." (account_code varchar(15), transdate DATE, transamount DOUBLE, transmemo TINYTEXT)";

        db_query($sql, "Can't create temporary table");
    }

    function drop_table()
    {
        $sql = "DROP TABLE IF EXISTS ".TB_PREF.$this->table;

        db_query($sql, "Can't drop temporary table");
    }

    function add_transaction($bank_account_gl_code, $transdate, $transamount, $transmemo)
    {
    	$acct = db_escape($bank_account_gl_code);
    	$date = db_escape($transdate);
    	$memo = db_escape($transmemo);
    	
        $sql = "INSERT INTO ".TB_PREF.$this->table." (account_code, transdate , transamount, transmemo) VALUES ($acct, $date, $transamount, $memo)";

        db_query($sql, "Can't create transaction row");
    }

	// modified version from includes/date_functions.inc
    function date2sql($date_,$how,$sep)
    {
        global $SysPrefs, $tmonths;
        /* takes a date in a the format specified in $DefaultDateFormat
        and converts to a yyyy/mm/dd format */

        //$how   = user_date_format();
        //$sep   = $SysPrefs->dateseps[user_date_sep()];
        $sep   = $SysPrefs->dateseps[$sep];

        $date_ = trim($date_);
        if ($date_ == null || strlen($date_) == 0)
        return "";

        $year = $month= $day  = 0;
        // Split up the date by the separator based on "how" to split it
        if ($how == 0 || $how == 3) // MMDDYYYY or MmmDDYYYY
        list($month, $day, $year) = explode($sep, $date_);
        elseif ($how == 1 || $how == 4) // DDMMYYYY or DDMmYYYY
        list($day, $month, $year) = explode($sep, $date_);
        else // $how == 2 || $how == 5, YYYYMMDD or YYYYMmmDD
        list($year, $month, $day) = explode($sep, $date_);

        if ($year + $day + $month) {
            if ($how > 2) {
                global $tmonths;
                $month = array_search($month, $tmonths);
            }
            //to modify assumption in 2030
            if ($SysPrefs->date_system == 0 || $SysPrefs->date_system == 3) {
                if ((int)$year < 60) {
                    $year = "20".$year;
                }
                elseif ((int)$year > 59 && (int)$year < 100) {
                    $year = "19".$year;
                }
            }
            if ((int)$year > 9999) {
                return 0;
            }
            if ($SysPrefs->date_system == 1)
            list($year, $month, $day) = jalali_to_gregorian($year, $month, $day);
            elseif ($SysPrefs->date_system == 2)
            list($year, $month, $day) = islamic_to_gregorian($year, $month, $day);
        }
        return sprintf("%04d-%02d-%02d", $year, $month, $day);
    }// end of function
}
