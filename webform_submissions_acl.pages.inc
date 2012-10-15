<?php
// $Id: webform_submissions_acl.pages.inc,v 1.1 2009/10/30 17:49:17 davyvandenbremt Exp $

/**
 * @file
 * ACL form.
 *
 * @author Davy Van Den Bremt
 * Modified by: Edouard Poitras <epoitras@layer8.org> on June 27th, 2012 in an attempt to
 * get it working with Drupal 7
 */
function webform_submissions_acl_page($form_state, $node) {
  $node_users = webform_submissions_acl_users_by_node($node->nid);
  $form = drupal_render(drupal_get_form('webform_submissions_acl_form', $node));
  return theme('webform_submissions_acl_page', array('form' => $form, 'node' => $node, 'users' => $node_users));
}

function theme_webform_submissions_acl_page($variables) {
  $form = $variables['form'];
  $node = $variables['node'];
  $users = $variables['users'];
  $header = array(t('Username'), t('Operations'));
  $rows = array();
  foreach ($users as $user) {
    $rows[] = array(l($user->name, 'user/'. $user->uid), l(t('Delete'), 'node/'. $node->nid .'/acl/delete/'. $user->uid));
  }
  if (empty($rows)) {
    $rows[] = array(array('data' => t('No users added yet.'), 'colspan' => '2', 'class' => 'message'));
  }
  return $form . theme('table', array('header' => $header, 'rows' => $rows));
}

function webform_submissions_acl_form($node, &$form_state) {
  $nid = $form_state['build_info']['args'][0]->nid;
  $form = array();
  $form['new_user'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add new user'),
  );
  $form['new_user']['nid'] = array(
    '#type' => 'hidden',
    '#default_value' => $nid,
  );
  $form['new_user']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#autocomplete_path' => 'user/autocomplete',
    '#size' => 40,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['new_user']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );
  return $form;
}

function webform_submissions_acl_form_validate($form, &$form_state) {
  $node_users = webform_submissions_acl_users_by_node($form_state['values']['nid'], TRUE);
  $account = array_shift(user_load_multiple(array(), array('name' => $form_state['values']['name'])));
  if (in_array($account->uid, $node_users)) {
    form_set_error('name', t('User is already added for this form'));
  }
  $account = array_shift(user_load_multiple(array(), array('name' => $form_state['values']['name'])));
  if (!$account) {
    form_set_error('name', t('Invalid user'));
  }
}

function webform_submissions_acl_form_submit($form, &$form_state) {
  $account = array_shift(user_load_multiple(array(), array('name' => $form_state['values']['name'])));
  if ($account) {
    $id = db_insert('webform_submissions_acl')->fields(array( 'nid' => $form_state['values']['nid'], 'uid' => $account->uid))->execute();
    drupal_set_message(t('The user was succesfully added.'), 'status');
  }
  else {
    drupal_set_message(t('Could not add user.'), 'error');
  }
}

/**
 * Menu callback -- Ask for confirmation of acl deletion
 */
function webform_submissions_acl_delete_confirm_form($form, &$form_state, $node, $account) {
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );
  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $account->uid,
  );
  return confirm_form($form,
    t('Are you sure you want to delete access rights for %user from %node?', array('%user' => $account->name, '%node' => $node->title)),
    isset($_GET['destination']) ? $_GET['destination'] : 'node/'. $node->nid .'/acl',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for executing section acl after confirmation
 */
function webform_submissions_acl_delete_confirm_form_submit($form, &$form_state) {
  $nid = $form_state['build_info']['args'][0]->nid;
  $uid = $form_state['build_info']['args'][1]->uid;
  webform_submissions_acl_delete($nid, $uid);
  $form_state['redirect'] = 'node/'. $nid .'/acl';
}
