<?php
/**********************************************************************
// Creator: Paul Shipley
// date_:   2019-01-09
// Title:   Import Bank Transactions
// Free software under GNU GPL
***********************************************************************/

class import_bank_transactions_app extends application
{

	function __construct()
	{
		$this->add_lapp_function(0, _("Import Bank Transactions"), 
			'/modules/import_bank_transactions/import_bank_transactions.php?', 'SA_IMPORTBANKTRANSACTIONS', MENU_TRANSACTION);

        $this->add_extensions();

	}

}
