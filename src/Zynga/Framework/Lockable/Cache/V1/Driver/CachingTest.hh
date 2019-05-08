<?hh // strict

namespace Zynga\Framework\Lockable\Cache\V1\Driver;

use Zynga\Framework\Cache\V2\Factory as CacheFactory;
use Zynga\Framework\Lockable\Cache\V1\Factory as LockableCacheFactory;
use
  Zynga\Framework\Lockable\Cache\V1\Interfaces\DriverConfigInterface as LockableCacheDriverConfigInterface
;
use
  Zynga\Framework\Lockable\Cache\V1\Interfaces\DriverInterface as LockableCacheDriverInterface
;
use Zynga\Framework\Lockable\Cache\V1\Test\Mock\SimpleStorable;
use Zynga\Framework\Lockable\Cache\V1\Test\Mock\PgDataExample;
use Zynga\Framework\Testing\TestCase\V2\Base as TestCase;

class CachingTest extends TestCase {
  private Vector<string> $_cacheNames = Vector {'Mock', 'PgDataTest'};

  public function doSetupBeforeClass(): bool {

    CacheFactory::disableMockDrivers();
    LockableCacheFactory::disableMockDrivers();

    CacheFactory::clear();
    LockableCacheFactory::clear();

    return true;

  }

  public function doTearDownAfterClass(): bool {

    CacheFactory::clear();
    LockableCacheFactory::clear();

    CacheFactory::enableMockDrivers();
    LockableCacheFactory::enableMockDrivers();

    return true;

  }

  public function testDrivers_Load(): void {

    foreach ($this->_cacheNames as $cacheName) {

      $instance = LockableCacheFactory::factory(
        LockableCacheDriverInterface::class,
        $cacheName,
      );

      $this->assertInstanceOf(LockableCacheDriverInterface::class, $instance);

    }

  }

  public function testDrivers_ConfigsOK(): void {

    foreach ($this->_cacheNames as $cacheName) {

      $instance = LockableCacheFactory::factory(
        LockableCacheDriverInterface::class,
        $cacheName,
      );

      $config = $instance->getConfig();

      $this->assertInstanceOf(
        LockableCacheDriverConfigInterface::class,
        $config,
      );

    }

  }

  public function test_LockUnlock_Mock(): void {

    $mockCache = LockableCacheFactory::factory(
      LockableCacheDriverInterface::class,
      'Mock',
    );

    $mockObj = new SimpleStorable();
    $mockObj->example_uint64->set(time() + mt_rand());

    // Lock the object
    $this->assertTrue($mockCache->lock($mockObj));
    // Should still be locked
    $this->assertTrue($mockCache->lock($mockObj));

    // Unlock the object
    $this->assertTrue($mockCache->unlock($mockObj));
    // We should be unlocked
    $this->assertTrue($mockCache->unlock($mockObj));

  }

  public function test_LockUnlock_pgData(): void {
    $pgDataCache = LockableCacheFactory::factory(
      LockableCacheDriverInterface::class,
      'PgDataTest',
    );

    $pgMock = new PgDataExample();
    $pgMock->id->set(time() + mt_rand());

    // lock the object
    $this->assertTrue($pgDataCache->lock($pgMock));

    // Shoudl still be locked
    $this->assertTrue($pgDataCache->lock($pgMock));

    // Unlock the object
    $this->assertTrue($pgDataCache->unlock($pgMock));

    // Should be unlocked
    $this->assertTrue($pgDataCache->unlock($pgMock));

  }

  public function test_GetSetDeleteCycle_Mock(): void {

    $mockCache = LockableCacheFactory::factory(
      LockableCacheDriverInterface::class,
      'Mock',
    );

    $mockId = time() + mt_rand();
    $mockTextValue = 'this-is-a-mock-text-value-'.mt_rand();

    $mockObj = new SimpleStorable();
    $mockObj->example_uint64->set($mockId);
    $mockObj->example_string->set($mockTextValue);

    // should be null as the item doesn't exist
    $this->assertEquals(null, $mockCache->get($mockObj));

    // save to cache
    $this->assertTrue($mockCache->set($mockObj));

    $returnObj = $mockCache->get($mockObj);

    if ($returnObj instanceof SimpleStorable) {
      $this->assertEquals($mockId, $returnObj->example_uint64->get());
      $this->assertEquals($mockTextValue, $returnObj->example_string->get());
    } else {
      $this->fail(
        'We should of seen SimpleStorable as a result from mockCache->get',
      );
    }

    // delete from cache
    $this->assertTrue($mockCache->delete($mockObj));

    // Test that it was indeed deleted.
    $this->assertEquals(null, $mockCache->get($mockObj));

  }

  public function test_GetSetDeleteCycle_pgData(): void {

    $pgDataCache = LockableCacheFactory::factory(
      LockableCacheDriverInterface::class,
      'PgDataTest',
    );

    $mockId = time() + mt_rand();
    $mockTextValue = 'this-is-a-pgdata-text-value-'.mt_rand();

    $pgMock = new PgDataExample();
    $pgMock->id->set($mockId);
    $pgMock->name->set($mockTextValue);

    // should be null as the item doesn't exist
    $this->assertEquals(null, $pgDataCache->get($pgMock));

    // save to cache
    $this->assertTrue($pgDataCache->set($pgMock));

    $returnObj = $pgDataCache->get($pgMock);

    if ($returnObj instanceof PgDataExample) {
      $this->assertEquals($mockId, $returnObj->id->get());
      $this->assertEquals($mockTextValue, $returnObj->name->get());
    } else {
      $this->fail(
        'We should of seen SimpleStorable as a result from pgDataCache->get',
      );
    }

    // delete from cache
    $this->assertTrue($pgDataCache->delete($pgMock));

    // Test that it was indeed deleted.
    $this->assertEquals(null, $pgDataCache->get($pgMock));

  }

}