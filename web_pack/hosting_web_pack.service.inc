<?php
/**
 * @file
 * Hosting service class for the Hosting web pack module.
 */

module_load_include('service.inc', 'hosting_web_server');

class hostingService_http_pack extends hostingService_http {
  public $type = 'pack';

  function view(&$render) {
    $render['master_servers'] = array(
      '#type' => 'item',
      '#title' => t('Master web servers'),
      '#markup' => theme('item_list', array('items' => array_map('_hosting_node_link', array_intersect_key($this->master_servers, hosting_get_servers('http'))))),
    );
    $render['slave_servers'] = array(
      '#type' => 'item',
      '#title' => t('Slave web servers'),
      '#markup' => theme('item_list', array('items' => array_map('_hosting_node_link', array_intersect_key($this->slave_servers, hosting_get_servers('http'))))),
    );
  }

  function form(&$form) {
    parent::form($form);

    $servers = hosting_get_servers('http');
    if (isset($this->server->nid)) {
      unset($servers[$this->server->nid]);
    }

    $form['master_servers'] = array(
      '#title' => t('Master servers'),
      '#type' => 'checkboxes',
      '#options' => $servers,
      '#default_value' => isset($this->master_servers) ? $this->master_servers : array(),
    );
    $form['slave_servers'] = array(
      '#title' => t('Slave servers'),
      '#type' => 'checkboxes',
      '#options' => $servers,
      '#default_value' => isset($this->slave_servers) ? $this->slave_servers : array(),
    );
  }

  function load() {
    parent::load();

    $ssl_enabled = TRUE;

    $master_servers = array();
    $slave_servers = array();
    $result = db_query('SELECT web_server_nid, type FROM {hosting_web_pack} WHERE vid = :vid', array(':vid' => $this->server->vid));
    while ($web_server = $result->fetch()) {
      if ($web_server->type == 'master') {
        $master_servers[] = $web_server->web_server_nid;
      }
      else {
        $slave_servers[] = $web_server->web_server_nid;
      }
      $server = node_load($web_server->web_server_nid);
      $ssl_enabled = $ssl_enabled && $server->services['http']->ssl_enabled;
    }

    $this->ssl_enabled = $ssl_enabled;
    $this->setValues(array('master_servers' => drupal_map_assoc($master_servers)));
    $this->setValues(array('slave_servers' => drupal_map_assoc($slave_servers)));
  }

  function insert() {
    parent::insert();
    foreach (array_filter($this->master_servers) as $web_server) {
      $record = array(
        'nid' => $this->server->nid,
        'vid' => $this->server->vid,
        'web_server_nid' => $web_server,
        'type' => 'master',
      );
      drupal_write_record('hosting_web_pack', $record);
    }
    foreach (array_filter($this->slave_servers) as $web_server) {
      $record = array(
        'nid' => $this->server->nid,
        'vid' => $this->server->vid,
        'web_server_nid' => $web_server,
        'type' => 'slave',
      );
      drupal_write_record('hosting_web_pack', $record);
    }
  }

  function delete() {
    parent::delete();
    db_delete('hosting_web_pack')
      ->condition('nid', $this->server->nid)
      ->execute();
  }

  function delete_revision() {
    parent::delete_revision();
    db_delete('hosting_web_pack')
      ->condition('vid', $this->server->vid)
      ->execute();
  }

  function context_options($task_type, $ref_type, &$task) {
    parent::context_options($task_type, $ref_type, $task);

    $task->context_options['master_web_servers'] = implode(',', array_values(array_map('hosting_context_name', array_intersect_key($this->master_servers, hosting_get_servers('http')))));
    $task->context_options['slave_web_servers'] = implode(',', array_values(array_map('hosting_context_name', array_intersect_key($this->slave_servers, hosting_get_servers('http')))));
  }
}
