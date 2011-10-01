/*
    Copyright (c) 2009 Grégory Oestreicher <greg@kamago.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "configdialog.h"
#include "settings.h"
#include "urlconfigurationdialog.h"

#include <kconfigdialogmanager.h>
#include <kconfigskeleton.h>
#include <klocale.h>

#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QStringList>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>

ConfigDialog::ConfigDialog( QWidget *parent )
  : KDialog( parent )
{
  mUi.setupUi( mainWidget() );

  mModel = new QStandardItemModel();
  QStringList headers;
  headers << i18n( "Protocol" ) << i18n( "URL" );
  mModel->setHorizontalHeaderLabels( headers );

  mUi.configuredUrls->setModel( mModel );
  mUi.configuredUrls->setRootIsDecorated( false );

  foreach ( const DavUtils::DavUrl &url, Settings::self()->configuredDavUrls() ) {
    KUrl displayUrl = url.url();
    displayUrl.setUser( QString() );
    addModelRow( DavUtils::protocolName( url.protocol() ), displayUrl.prettyUrl() );
  }

  mManager = new KConfigDialogManager( this, Settings::self() );
  mManager->updateWidgets();

  connect( mUi.kcfg_displayName, SIGNAL(textChanged(QString)), this, SLOT(checkUserInput()) );
  connect( mUi.configuredUrls->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this, SLOT(checkConfiguredUrlsButtonsState()) );

  connect( mUi.addButton, SIGNAL(clicked()), this, SLOT(onAddButtonClicked()) );
  connect( mUi.removeButton, SIGNAL(clicked()), this, SLOT(onRemoveButtonClicked()) );
  connect( mUi.editButton, SIGNAL(clicked()), this, SLOT(onEditButtonClicked()) );

  connect( this, SIGNAL(okClicked()), this, SLOT(onOkClicked()) );
  connect( this, SIGNAL(cancelClicked()), this, SLOT(onCancelClicked()) );

  checkUserInput();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::checkUserInput()
{
  checkConfiguredUrlsButtonsState();

  if ( !mUi.kcfg_displayName->text().isEmpty() && !( mModel->invisibleRootItem()->rowCount() == 0 ) )
    enableButtonOk( true );
  else
    enableButtonOk( false );
}

void ConfigDialog::onAddButtonClicked()
{
  QPointer<UrlConfigurationDialog> dlg = new UrlConfigurationDialog( this );
  const int result = dlg->exec();

  if ( result == QDialog::Accepted && !dlg.isNull() ) {
    Settings::UrlConfiguration *urlConfig = new Settings::UrlConfiguration();

    urlConfig->mUrl = dlg->remoteUrl();
    urlConfig->mUser = dlg->username();
    urlConfig->mPassword = dlg->password();
    urlConfig->mProtocol = dlg->protocol();

    Settings::self()->newUrlConfiguration( urlConfig );

    const QString protocolName = DavUtils::protocolName( dlg->protocol() );

    addModelRow( protocolName, dlg->remoteUrl() );
    mAddedUrls << QPair<QString, DavUtils::Protocol>( dlg->remoteUrl(), DavUtils::Protocol( dlg->protocol() ) );
    checkUserInput();
  }

  delete dlg;
}

void ConfigDialog::onRemoveButtonClicked()
{
  const QModelIndexList indexes = mUi.configuredUrls->selectionModel()->selectedRows();
  if ( indexes.size() == 0 )
    return;

  QString proto = mModel->index( indexes.at( 0 ).row(), 0 ).data().toString();
  QString url = mModel->index( indexes.at( 0 ).row(), 1 ).data().toString();

  mRemovedUrls << QPair<QString, DavUtils::Protocol>( url, DavUtils::protocolByName( proto ) );
  mModel->removeRow( indexes.at( 0 ).row() );

  checkUserInput();
}

void ConfigDialog::onEditButtonClicked()
{
  const QModelIndexList indexes = mUi.configuredUrls->selectionModel()->selectedRows();
  if ( indexes.size() == 0 )
    return;

  const QString proto = mModel->index( indexes.at( 0 ).row(), 0 ).data().toString();
  const QString url = mModel->index( indexes.at( 0 ).row(), 1 ).data().toString();

  Settings::UrlConfiguration *urlConfig = Settings::self()->urlConfiguration( DavUtils::protocolByName( proto ), url );
  if ( !urlConfig )
    return;

  QPointer<UrlConfigurationDialog> dlg = new UrlConfigurationDialog( this );
  dlg->setRemoteUrl( urlConfig->mUrl );
  dlg->setProtocol( DavUtils::Protocol( urlConfig->mProtocol ) );
  dlg->setUsername( urlConfig->mUser );
  dlg->setPassword( urlConfig->mPassword );

  const int result = dlg->exec();

  if ( result == QDialog::Accepted && !dlg.isNull() ) {
    Settings::self()->removeUrlConfiguration( DavUtils::protocolByName( proto ), url );
    Settings::UrlConfiguration *urlConfigAccepted = new Settings::UrlConfiguration();
    urlConfigAccepted->mUrl = dlg->remoteUrl();
    urlConfigAccepted->mUser = dlg->username();
    urlConfigAccepted->mPassword = dlg->password();
    urlConfigAccepted->mProtocol = dlg->protocol();
    Settings::self()->newUrlConfiguration( urlConfigAccepted );

    QStandardItem *item = mModel->item( indexes.at( 0 ).row(), 0 ); // Protocol
    item->setData( QVariant::fromValue( DavUtils::protocolName( dlg->protocol() ) ), Qt::DisplayRole );

    item = mModel->item( indexes.at( 0 ).row(), 1 ); // URL
    item->setData( QVariant::fromValue( dlg->remoteUrl() ), Qt::DisplayRole );
  }
  delete dlg;
}

void ConfigDialog::onOkClicked()
{
  QPair<QString, DavUtils::Protocol> url;
  foreach ( url, mRemovedUrls )
    Settings::self()->removeUrlConfiguration( url.second, url.first );

  mManager->updateSettings();
}

void ConfigDialog::onCancelClicked()
{
  mRemovedUrls.clear();

  QPair<QString, DavUtils::Protocol> url;
  foreach ( url, mAddedUrls )
    Settings::self()->removeUrlConfiguration( url.second, url.first );
}

void ConfigDialog::checkConfiguredUrlsButtonsState()
{
  const bool enabled = mUi.configuredUrls->selectionModel()->hasSelection();

  mUi.removeButton->setEnabled( enabled );
  mUi.editButton->setEnabled( enabled );
}

void ConfigDialog::addModelRow( const QString &protocol, const QString &url )
{
  QStandardItem *rootItem = mModel->invisibleRootItem();
  QList<QStandardItem*> items;

  QStandardItem *protocolStandardItem = new QStandardItem( protocol );
  protocolStandardItem->setEditable( false );
  items << protocolStandardItem;

  QStandardItem *urlStandardItem = new QStandardItem( url );
  urlStandardItem->setEditable( false );
  items << urlStandardItem;

  rootItem->appendRow( items );
}

#include "configdialog.moc"
