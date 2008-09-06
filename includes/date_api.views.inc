<?php
//$Id$

/**
 * @file
 * Defines date-related Views data and plugins:
 * 
 * Date argument:
 *   A generic date argument that has an option to select one or more 
 *   Views date fields to filter on, automatically adds them to the view, 
 *   and then filters the view by the value of the selected field(s). 
 *   The flexible argument will accept and evaluate most ISO date
 *   and period formats, like 2009-05-01, 2008-W25, P1W. 
 * 
 * Date filter:
 *   A generic date filter that has an option to select a 
 *   Views date field to filter on, with a choice of a widget to use
 *   for the filter form and an option to set the default value to 
 *   a set date or something like 'now +90 days'. If the operator is 
 *   set to 'between' or 'not between' you can set a default value for
 *   both the from and to dates. 
 * 
 * Current date argument default
 *   Adds a default option to set the argument to the current date 
 *   when the argument is empty.
 * 
 * Date navigation attachment
 *   Navigation that can be attached to any display to create back/next 
 *   links by date, requires the date argument and uses the current
 *   date argument default to set a starting point for the view.
 */
/**
 * Implementation of hook_views_handlers() to register all of the basic handlers
 * views uses.
 */
function date_api_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'date_api') .'/includes',
      ),
    'handlers' => array(
      'date_api_argument_handler' => array(
        'parent' => 'views_handler_argument_date',
      ),
      'date_api_filter_handler' => array(
        'parent' => 'views_handler_filter_numeric',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_plugins
 */
function date_api_views_plugins() {
  $path = drupal_get_path('module', 'date_api') .'/theme';
  $file = 'theme.inc';
  require_once "./$path/$file";
  return array(
    'module' => 'date_api', // This just tells our themes are elsewhere.
    'display' => array(
      // Display plugin for date navigation.
      'date_nav' => array(
        'title' => t('Date browser'),
        'help' => t('Date back/next navigation to attach to other displays. Requires the Date argument.'),
        'handler' => 'date_plugin_display_attachment',
        'parent' => 'views_plugin_display_attachment',
        'theme' => 'views_view',
        'use ajax' => TRUE,
        'admin' => t('Date browser'),
        'help topic' => 'display-date_navigation',
      ),
    ),
    'style' => array(
      // Style plugin for the navigation display.
      'date_nav' => array(
        'title' => t('Date browser style'),
        'help' => t('Creates back/next navigation.'),
        'handler' => 'date_navigation_plugin_style',
        'parent' => 'views_plugin_style',
        'theme' => 'date_navigation',
        'theme file' => $file,
        'theme path' => $path,
        'uses row plugin' => FALSE,
        'uses fields' => FALSE,
        'uses options' => TRUE,
        'type' => 'date_nav', 
        'even empty' => TRUE,
      ),
    ),
  );
}

/**
 * Implementation of hook_views_data()
 */
function date_api_views_data() {
  $data = array();
  
  // The flexible date argument.
  $data['node']['date_argument'] = array(
    'group' => t('Date'),
    'title' => t('Date'),
    'help' => t('Filter any Views date field by a date argument, using any common ISO date/period format (i.e. YYYY, YYYY-MM, YYYY-MM-DD, YYYY-W99, YYYY-MM-DD--P3M, P90D, etc).'),
    'argument' => array(
      'handler' => 'date_api_argument_handler',
      'empty name field' => t('Undated'),
    ),
  );  
  // The flexible date fliter.
  $data['node']['date_filter'] = array(
    'group' => t('Date'),
    'title' => t('Date'),
    'help' => t('Filter any Views date field.'),
    'filter' => array(
      'handler' => 'date_api_filter_handler',
      'empty name field' => t('Undated'),
    ),
  ); 
  return $data;
}

/**
 *  Identify all potential date/timestamp fields and cache the data.
 */
function date_api_fields($base = 'node', $reset = FALSE) {
  static $fields;
  require_once('./'. drupal_get_path('module', 'date_api') .'/includes/date_api_fields.inc');
  if (empty($fields[$base]) || $reset) {
    $cid = 'date_api_fields_'. $base;
    if (!$reset && $cached = cache_get($cid, 'cache_views')) {
      $fields[$base] = $cached->data;
    }
    else {
      $fields[$base] = _date_api_fields($base);
    }
  }
  return $fields[$base];
}

/**
 * Central function for setting up the right timezone values
 * in the SQL date handler.
 * 
 * The date handler will use this information to decide if the
 * database value needs a timezone conversion.
 * 
 * In Views, we will always be comparing to a local date value,
 * so the goal is to convert the database value to the right
 * value to compare to the local value.
 */
function date_views_set_timezone(&$date_handler, &$view, $field) {
  $tz_handling  = $field['tz_handling'];
  switch ($tz_handling) {
    case 'date' :
      $date_handler->db_timezone = 'UTC';
      $date_handler->local_timezone_field = $field['timezone_field'];
      $date_handler->local_offset_field = $field['offset_field'];
      break;
    case 'none':
      $date_handler->db_timezone = date_default_timezone_name();
      $date_handler->local_timezone = date_default_timezone_name();
      break;
    case 'utc':
      $date_handler->db_timezone = 'UTC';
      $date_handler->local_timezone = 'UTC';
      break;
    default :
      $date_handler->db_timezone = 'UTC';
      $date_handler->local_timezone = date_default_timezone_name();
      break;
  }
}